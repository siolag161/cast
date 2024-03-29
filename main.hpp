/****************************************************************************************
 * File: main.hpp
 * Description: 
 * @author: siolag161 (thanh.phan@outlook.com)
 * @date: 29/04/2014

 ***************************************************************************************/
#ifndef _MAIN_HPP
#define _MAIN_HPP

#include "Matrix.hpp"
#include "CAST.hpp"

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

inline std::shared_ptr<Matrix> loadDataTable( const std::string& file_name,
                                              const char& sep = ',',
                                              const char& quote = '"' );

///////////////////////////////////////////////////////
inline void loadLabelPosition( std::vector<Label>& labels,
                               std::vector<int>& positions,
                               const std::string& infile );

/////////////////////////////////////////////////////// 
void checkInputFile(const std::string& inputFileName);
ApplicationOptions getProgramOptions(int argc, char** argv);


void saveClustering( const fltm::CAST_Partition& clustering, const unsigned currentSize, std::string clustFN ) ;
void saveStatistics( const fltm::CAST_Partition& clustering, const float score, const float elapsedTime, std::string statisticFN );
////////////////////////////// RANGE_ITERATOR ///////////////////////////////////////////
struct Clust_CAST_Opt {
  Clust_CAST_Opt(unsigned mD, float s, float c): maxDist(mD), simi(s), cast(c) {}
  unsigned maxDist; float simi; float cast;
};

/****************************************************************************************/
#endif // _MAIN_HPP
