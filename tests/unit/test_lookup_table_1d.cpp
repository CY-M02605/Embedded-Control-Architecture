/**
 * @file test_lookup_table1.cpp
 * @brief Tests the LookupTable1D utility, including boundary handling and linear interpolation
 * @date 2026-07-02 
 */

#include "lookup_table_1d.h"

#include <cassert>
#include <iostream>

int main() {
    using Table = utility::LookupTable1D<float>;

    const Table::Point table_point[] = {
        {10.0f, 1.0f},
        {20.0f, 2.0f},
        {30.0f, 3.0f},
        {40.0f, 4.0f},
        {50.0f, 5.0f}
    };

    std::size_t table_size = sizeof(table_point) / sizeof(table_point[0]);

    Table table(table_point, table_size);

    assert(table.LookupTable(5.0f) == 1.0f);
    assert(table.LookupTable(10.0f) == 1.0f);
    assert(table.LookupTable(15.0f) == 1.5f);

    // std::cout << "table.LookupTable(15.0f) = " << table.LookupTable(15.0f) << std::endl;
    assert(table.LookupTable(30.0f) == 3.0f);
    assert(table.LookupTable(50.0f) == 5.0f);
    assert(table.LookupTable(60.0f) == 5.0f);

    return 0;
}
