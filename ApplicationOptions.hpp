/****************************************************************************************
 * File: ApplicationOptions.hpp
 * Description: 
 * @author: siolag161 (thanh.phan@outlook.com)
 * @date: 28/04/2014

 ***************************************************************************************/
#ifndef _APPLICATIONOPTIONS_HPP
#define _APPLICATIONOPTIONS_HPP

struct ApplicationOptions
{
  std::string dataInFile; // input filename
  std::string labPosInFile; // input filename
  
  int hasHeader; // indicates whether the input file contains a header row

  double thresCAST; // threshold for CAST algorithm computation
  double thresSimMatrix; // threshold for the similarity matrix, necessary for the execution of FHLCM

  unsigned maxDistance;

  ApplicationOptions(): hasHeader(true) {}
};

/****************************************************************************************/
#endif // _APPLICATIONOPTIONS_HPP
