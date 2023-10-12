#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <vector>

class Sheet : public SheetInterface {
public:
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

	// Можете дополнить ваш класс нужными полями и методами


private:
	// Можете дополнить ваш класс нужными полями и методами
    std::vector<std::vector<std::unique_ptr<CellInterface>>> data_;
    Size printed_part_;
    void Print(const CellInterface::Value& value, std::ostream& output) const;
    void PrintedPartSize(Position pos);
    std::vector<int> GetNonEmptyCelOnRow(int index_row);
    std::vector<int> GetNonEmptyCelOnCol(int index_col);
};