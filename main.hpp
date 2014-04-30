/****************************************************************************************
 * File: main.hpp
 * Description: 
 * @author: siolag161 (thanh.phan@outlook.com)
 * @date: 29/04/2014

 ***************************************************************************************/
#ifndef _MAIN_HPP
#define _MAIN_HPP

#include "Matrix.hpp"
class ApplicationOptions;

//typedef fltm::Matrix<int> Matrix;
// typedef std::vector< int> Row;
// typedef std::vector< Row > Matrix;
typedef std::string Label;
typedef int Position;

struct ConstraintMap {
  unsigned int operator()(int varA, int varB) {
    return abs( positions[varA] - positions[varB] );
  }
  std::vector<int>& positions;
  ConstraintMap(std::vector<int>& pos): positions(pos) {}
};

typedef std::vector < std::vector <int> > Matrix;

 inline Matrix* loadDataTable( const std::string& file_name,
     const bool& hasHeader,
     const char& sep = ',',
     const char& quote = '"');

///////////////////////////////////////////////////////
inline void loadLabelPosition( std::vector<Label>& labels,
                               std::vector<int>& positions,
                               const std::string& infile );

/////////////////////////////////////////////////////// 
void checkInputFile(const std::string& inputFileName);

ApplicationOptions getProgramOptions(int argc, char** argv);

/** reminder: by default sep = ',' while quote = '"'
 */
// Matrix loadDataTable( const std::string& infile,
//                       const bool& hasHeader,
//                       const char& sep,
//                       const char& quote);

void loadLabelPosition( std::vector<Label> & labels,
                        std::vector<Position>& positions,
                        const std::string& infile );

/****************************************************************************************/
#endif // _MAIN_HPP
