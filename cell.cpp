#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>

using namespace std::literals;


CellInterface::Value EmptyImpl::GetValue() const {
	return std::string();
}

std::string EmptyImpl::GetText() const {
	return std::string();
}

void EmptyImpl::Clear() {}



TextImpl::TextImpl(std::string text) {
	if (text.size() == 0) {
		text_ = ""s;
	}
	else {
		text_ = text;
	}
}

CellInterface::Value TextImpl::GetValue() const {
	if (text_.at(0) == '\'') {
		return text_.substr(1);
	}
	return text_;
}

std::string TextImpl::GetText() const {
	return text_;
}

void TextImpl::Clear() {
	text_.clear();
}



FormulaImpl::FormulaImpl(const std::string& formula, const SheetInterface& sheet) : formula_(ParseFormula(formula)), table_(sheet) {}

CellInterface::Value FormulaImpl::GetValue() const {
	CellInterface::Value buffer;
	auto result = formula_->Evaluate(table_);

	if (std::holds_alternative<double>(result)) {
		buffer = std::get<double>(result);
	}
	else {
		buffer = std::get<FormulaError>(result);
	}

	return buffer;
}

std::string FormulaImpl::GetText() const {
	return  std::string("="s) + formula_->GetExpression();
}

void FormulaImpl::Clear() {
	formula_ = ParseFormula(""s);
}

std::vector<Position> FormulaImpl::GetReferencedCells() const {
	return formula_->GetReferencedCells();
}


Cell::Cell(Position pos, SheetInterface& sheet) :table_(&sheet), position_(pos){}

void Cell::Set(std::string text) {

	if (text.size() > 0) {
		if (text.at(0) == '=' && text.size() > 1) {
			auto buffer = std::make_unique<FormulaImpl>(text.substr(1), *table_);

			std::vector<Position> reference = buffer->GetReferencedCells();

			for (const auto& pos : reference) {
				if (!pos.IsValid()) {
					throw FormulaException("Wrong adress");
				}
			}

			CheckCyclicDependencies(reference);

			curr_point_to = std::move(reference);
			impl_ = std::move(buffer);
		}
		else {
			impl_ = std::make_unique<TextImpl>(text);
		}
	}
	else {
		impl_ = std::make_unique<EmptyImpl>();
	}

	InvalidateCache();
	InformChilds();
}

void Cell::Clear() {
	impl_->Clear();

	InvalidateCache();
}

Cell::Value Cell::GetValue() const {
	if (HasCache()) {                             
		return cache_.value();                    
	}

	auto value = impl_->GetValue();               

	if (std::holds_alternative<double>(value)) {  
		cache_ = std::get<double>(value);
	}

	return value;
}

std::string Cell::GetText() const {
	return impl_->GetText();
}

bool Cell::HasCache() const {
	return cache_.has_value();
}

void Cell::ClearCache() {
	cache_.reset();
}

std::vector<Position> Cell::GetReferencedCells() const {
	return curr_point_to;
}

std::vector<Position> Cell::GetCacheCells() const {
	return point_to_curr;
}

void Cell::InvalidateCache() {
	if (point_to_curr.size() != 0) {                                
		for (const auto& cell_pos : point_to_curr) {                
			Cell* cell = dynamic_cast<Cell*>(table_->GetCell(cell_pos));  
			cell->ClearCache();                                           
			cell->InvalidateCache();                                      
		}
	}
}

void Cell::InformChilds() {
	if (curr_point_to.size() != 0) {
		for (const auto& cell_pos : curr_point_to) {
			Cell* cell = dynamic_cast<Cell*>(table_->GetCell(cell_pos));
			cell->AddCacheCell(position_);
		}
	}
}

void Cell::AddCacheCell(Position pos) {
	point_to_curr.push_back(pos);
	std::sort(point_to_curr.begin(), point_to_curr.end());
}

void Cell::CheckCyclicDependencies(std::vector<Position>& cells) const {

	for (auto& cell_pos : cells) {
		if (cell_pos == position_) {
			throw CircularDependencyException("circular dependency");
		}

		Cell* cell = dynamic_cast<Cell*>(table_->GetCell(cell_pos));

		if (!cell) {
			table_->SetCell(cell_pos, "");
			cell = dynamic_cast<Cell*>(table_->GetCell(cell_pos));
		}

		cell->Checker(position_);
	}
}

void Cell::Checker(Position checked_position) const {

	if (curr_point_to.size() != 0) {                                 
		for (const auto& cell_pos : curr_point_to) {                 
			if (cell_pos == checked_position) {                           
				throw CircularDependencyException("circular dependency"); 
			}

			Cell* cell = dynamic_cast<Cell*>(table_->GetCell(cell_pos));  

			cell->Checker(checked_position);                        
		}
	}
}