#pragma once

#ifndef INTERVAL_H
#define INTERVAL_H

class Interval {
public:
    double min, max;

    Interval() : min(+INFINITY), max(-INFINITY) {} // Default interval is empty

    Interval(double min, double max) : min(min), max(max) {}

    Interval(const Interval& a, const Interval& b)
    {
        min = a.min <= b.min ? a.min : b.min;
        max = a.max >= b.max ? a.max : b.max;
    }

    /// <summary>
    /// Gets the size of the interval.
    /// </summary>
    /// <returns></returns>
    double size() const
    {
        return max - min;
    }

    /// <summary>
    /// Returns true if some value x is in the interval. 
    /// x can also be equal to the min or max of the interval.
    /// </summary>
    /// <param name="x">:: double</param>
    /// <returns></returns>
    bool contains(double x) const
    {
        return min <= x && x <= max;
    }

    /// <summary>
    /// Returns true if some value x is in the interval.
    /// x cannot be equal to the min and max of the interval.
    /// </summary>
    /// <param name="x">:: double</param>
    /// <returns></returns>
    bool surrounds(double x) const
    {
        return min < x && x < max;
    }

    /// <summary>
    /// Clamps a value to the values within the interval.
    /// </summary>
    /// <param name="x">:: double</param>
    /// <returns></returns>
    double clamp(double x) const
    {
        if (x < min) return min;
        if (x > max) return max;
        return x;
    }

    bool overlap(const Interval i) const
    {
        return min <= i.max && i.min <= max;
    }

    static const Interval empty, universe;
};

const Interval Interval::empty = Interval(+INFINITY, -INFINITY);
const Interval Interval::universe = Interval(-INFINITY, +INFINITY);

#endif
