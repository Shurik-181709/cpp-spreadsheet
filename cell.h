#pragma once

#include "common.h"
#include "formula.h"

class Impl {
public:
    virtual CellInterface::Value GetValue() const = 0;

    virtual std::string GetText() const = 0;

    virtual void Clear() = 0;
};

class FormulaImpl : public Impl {
public:
    FormulaImpl(std::string formula);

    CellInterface::Value GetValue() const override;

    std::string GetText() const override;

    void Clear() override;

private:
    std::unique_ptr<FormulaInterface> value_;
};

class EmptyImpl : public Impl {
public:
    EmptyImpl() = default;

    CellInterface::Value GetValue() const override;

    std::string GetText() const override;

    void Clear() override;

};


class TextImpl : public Impl {
public:
    TextImpl(std::string text);

    CellInterface::Value GetValue() const override;

    std::string GetText() const override;

    void Clear() override;

private:
    std::string value_;
};

class Cell : public CellInterface {
public:
    Cell() = default;

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;

private:
    std::unique_ptr<Impl> impl_;
};