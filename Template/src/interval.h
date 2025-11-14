#pragma once

#ifndef INTERVAL_H
#define INTERVAL_H

class Interval {
public:
    double min, max;

    Interval() : min(+infinity), max(-infinity) {} // Default interval is empty

    Interval(double min, double max) : min(min), max(max) {}

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

    static const Interval empty, universe;
};

const Interval Interval::empty = Interval(+infinity, -infinity);
const Interval Interval::universe = Interval(-infinity, +infinity);

#endif
