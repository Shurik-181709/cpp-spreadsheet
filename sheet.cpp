#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>
#include <variant>

using namespace std::literals;


void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("invalid position");
    }

    if (data_.size() < size_t(pos.row + 1)) {
        int d = pos.row + 1 - data_.size();
        for (int i = 0; i < d; ++i) {
            data_.push_back(std::vector<std::unique_ptr< CellInterface>>());
        }
    }

    if (data_.at(pos.row).size() < size_t(pos.col + 1)) {
        int d = pos.col + 1 - data_.at(pos.row).size();
        for (int i = 0; i < d; ++i) {
            data_.at(pos.row).push_back(nullptr);
        }
    }

    std::unique_ptr < CellInterface > cell = std::make_unique<Cell>(pos, *this);
    cell->Set(text);
    data_.at(pos.row).at(pos.col) = std::move(cell);

    if (printed_part_.rows < pos.row + 1) {
        printed_part_.rows = pos.row + 1;
    }
    if (printed_part_.cols < pos.col + 1) {
        printed_part_.cols = pos.col + 1;
    }
}


const CellInterface* Sheet::GetCell(Position pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException("invalid position");
    }

    if (data_.size() < size_t(pos.row + 1) || data_.at(pos.row).size() < size_t(pos.col + 1)) {
        return nullptr;
    }

    return data_.at(pos.row).at(pos.col).get();
}


CellInterface* Sheet::GetCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("invalid position");
    }

    if (data_.size() < size_t(pos.row + 1) || data_.at(pos.row).size() < size_t(pos.col + 1)) {
        return nullptr;
    }

    return data_.at(pos.row).at(pos.col).get();
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("invalid position");
    }

    if (data_.size() < size_t(pos.row + 1) || data_.at(pos.row).size() < size_t(pos.col + 1)) {
        return;
    }

    data_.at(pos.row).at(pos.col).release();
    PrintedPartSize();
}


void Sheet::PrintedPartSize() {
    size_t maxrow = 0;
    size_t maxcol = 0;

    for (size_t i = 0; i < data_.size(); ++i) {
        size_t size = 0;
        for (size_t n = 0; n < data_.at(i).size(); ++n) {
            if (data_.at(i).at(n) != nullptr) {
                size = n + 1;
            }
        }

        if (size != 0) {
            maxrow = i + 1;
            if (maxcol < size) {
                maxcol = size;
            }

        }
    }

    printed_part_.rows = maxrow;
    printed_part_.cols = maxcol;
}


Size Sheet::GetPrintableSize() const {
    return printed_part_;
}


void Sheet::PrintValues(std::ostream& output) const {
    if (printed_part_.rows != 0 && printed_part_.cols != 0) {

        for (int row = 0; row < printed_part_.rows; ++row) {
            for (int col = 0; col < printed_part_.cols; ++col) {

                const CellInterface* cell = GetCell({ row,col });

                if (col != 0) {
                    output << "\t";
                }

                if (cell) {
                    const auto& value = cell->GetValue();
                    PrintValue(value, output);
                }
            }
            output << "\n";
        }
    }
}

void Sheet::PrintValue(const CellInterface::Value& value, std::ostream& output) const {
    if (std::holds_alternative<double>(value)) {
        output << std::get<double>(value);
    }
    else if (std::holds_alternative<std::string>(value)) {
        output << std::get<std::string>(value);
    }
    else {
        output << std::get<FormulaError>(value);
    }
}


void Sheet::PrintTexts(std::ostream& output) const {
    if (printed_part_.rows != 0 && printed_part_.cols != 0) {
        for (int row = 0; row < printed_part_.rows; ++row) {
            for (int col = 0; col < printed_part_.cols; ++col) {

                const CellInterface* cell = GetCell({ row,col });

                if (col != 0) {
                    output << "\t";
                }

                if (cell) {
                    output << data_.at(row).at(col)->GetText();
                }
            }
            output << "\n";
        }
    }

}


std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}