#ifndef NETLIB_EXAMPLES_SUDOKU_SUDOKU_H_
#define NETLIB_EXAMPLES_SUDOKU_SUDOKU_H_

#include <muduo/base/StringPiece.h>
#include <muduo/base/Types.h>

const int kCells = 81; // Cells' number.
extern const char kNoSolution[];
muduo::string SolveSudoku(const muduo::StringPiece& puzzle);

#endif // NETLIB_EXAMPLES_SUDOKU_SUDOKU_H_
