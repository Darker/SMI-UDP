#ifndef OFFSETRANGEMAP_H
#define OFFSETRANGEMAP_H
#include <QLinkedList>
#include <QString>

#define UINT_RANGE_ABS(x,y) (x>y?x-y:y-x)
class Range {
public:
    Range(quint32 from, quint32 to) : start(from), end(to) {}
    bool contains(const quint32 value) const {return value>=start && value<=end;}
    bool contains(const Range& value) const {return value.start>=start && value.end<=end;}
    bool overlaps(const Range& value) const {
        return contains(value.start) || contains(value.end) || value.contains(start) || value.contains(end);
    }
    // as when overlaps, but values exactly (eg. a+-1 = b) next to each other also mean true
    bool toutches(const Range value) const {
        return overlaps(value) || UINT_RANGE_ABS(start, value.end)==1 || UINT_RANGE_ABS(end, value.start)==1;
    }
    // There is no chech if ranges toutch, you have to do that yourself
    void add(const Range& other) {
        if(other.start<start)
            start = other.start;
        if(other.end>end) {
            end = other.end;
        }
    }

    quint32 start;
    quint32 end;
    Range operator+(const Range& other);
    bool operator==(const Range& other) const {return start==other.start && end==other.end;}
    bool operator>(const Range& other) const {end>other.end;}
    bool operator<(const Range& other) const {other>*this;}
    QString toString() const {return "<"+QString::number(start)+", "+QString::number(end)+">";}

};

/** This class maps what parts of file have been received and what not.
 * This is done using intervals stored in a list. **/
class OffsetRangeMap
{
public:
    OffsetRangeMap();
    bool containsValue(quint32 val) const;
    void addValue(quint32 val) {return addRange(val, val);}
    void addRange(quint32 from, quint32 to);
    // Return sum of values in the map
    quint32 sum() const;
    QString toString() const;
protected:
    QLinkedList<Range> data;
};

#endif // OFFSETRANGEMAP_H
