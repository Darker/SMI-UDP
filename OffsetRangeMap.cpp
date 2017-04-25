#include "OffsetRangeMap.h"

OffsetRangeMap::OffsetRangeMap()
{

}

bool OffsetRangeMap::containsValue(quint32 val) const
{
    Q_FOREACH(const Range& r, data) {
        if(val>=r.start && val<=r.end)
            return true;
    }
    return false;
}

void OffsetRangeMap::addRange(quint32 from, quint32 to)
{

    const Range r(from, to);
    //int previousIndex = -1;
    QLinkedList<Range>::iterator iterator=data.begin();
    QLinkedList<Range>::iterator end=data.end();
    for(; iterator!=end; ++iterator) {
        /*if(previousIndex>-1) {
            if(previousRange.overlaps(r)) {
                data[i] = previousRange+r;
                break;
            }
        }*/
        Range& current = *iterator;
        if(current.toutches(r)) {
            current.add(r);
            break;
        }
        // totally afar from each other
        // we can push the value here then
        else if(current.start>r.end) {
            iterator = data.insert(iterator, r);
            break;
        }
    }
    // If iterator is at the end no values were affected and we can append and quit
    if(iterator==end) {
        data.append(r);
        return;
    }

    // Go one value back, to be able to completely check all possibly affected values
    if(iterator!=data.begin())
        --iterator;
    // fix the rest of the list - remove any ranges that now overlap
    Range& previous = *iterator;
    ++iterator;

    for(; iterator!=end;) {
        Range& current = *iterator;
        if(current.toutches(previous)) {
            previous.add(current);
            iterator = data.erase(iterator);
            end = data.end();
        }
        // If no change is needed then we can end
        // obviously, ranges later in array must also be unaffected
        else {
            break;
        }
    }
}

quint32 OffsetRangeMap::sum() const
{
    quint32 sum = 0;
    Q_FOREACH(const Range& r, data) {
        sum += r.end-r.start+1;
    }
    return sum;
}

QString OffsetRangeMap::toString() const {
    QString result;
    Q_FOREACH(const Range& r, data) {
        result+=r.toString();
    }
    return result;
}

Range Range::operator+(const Range& other)
{
    return Range(start>other.start?other.start:start, end>other.end?end:other.end);
}
