#pragma once


class IBitboard {
public:
    virtual ~IBitboard() = default;

    virtual void clear() = 0;
    virtual bool isEmpty() const = 0;
    virtual int popcount() const = 0;

    virtual void setCell(int x, int y, bool value) = 0;
    virtual bool getCell(int x, int y) const = 0;

    virtual void merge(const IBitboard& other) = 0;
    virtual void applyObstacles(const IBitboard& obstacles) = 0;
};