
#include "TimerUtils.hpp"
#include "ApplicationOptions.hpp"
#include "OptionPrinter.hpp" //for printing
#include "DataTable.hpp"
#include "CSVParser.hpp"
#include "DataTransformation.hpp"

#include "main.hpp"

#include <iostream> 
#include <fstream>
#include <cstdio>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp> // to obtain the program's nam

#include "Clustering.hpp"
#include "CAST.hpp"
namespace utl = utility;
namespace po = boost::program_options;   

int main(int argc, char** argv) {

  utl::Timer timer; timer.start();
  const ApplicationOptions progOpt = getProgramOptions(argc, argv); // get user options

   
  std::vector<Label> labels; std::vector<Position> positions;
  loadLabelPosition( labels, positions, progOpt.labPosInFile );
  std::cout << "loading data from " <<  progOpt.dataInFile << std::endl; // todo: logging

  Matrix* matrix = loadDataTable ( progOpt.dataInFile, progOpt.hasHeader); // load data into the the dataMat
  std::cout << "data loaded. rows: " << utl::nrow(*matrix) << ", columns: "
            << utl::ncol(*matrix) << ". takes: " <<  timer.display() << std::endl << std::endl; // todo: logging
  timer.restart();
 
   std::vector<unsigned> labelMap; for (unsigned i = 0; i < labels.size(); ++i) labelMap.push_back(i);


  fltm::SimilarityCompute* simiCompute = new fltm::SimilarityCompute( positions, matrix,  progOpt.thresSimMatrix, progOpt.maxDistance );
      
  timer.restart();
  fltm::CAST clusteringCAST (progOpt.thresCAST) ;
  std::cout << "start clustering..." << std::endl;
  fltm::CAST_Partition* clustering = clusteringCAST( simiCompute, labelMap );
   std::cout << "end clustering. gives: " << clustering->size() 
             << ". takes: " <<  timer.display() << std::endl;

   // delete matrix;
   // delete simiCompute;
   getchar();
  // clt::CAST<unsigned, unsigned> clusteringCAST( progOpt.thresCAST );  
  // ClusterResultPointer clustering = clusteringCAST( simiMat, labelMap );  

  // std::cout << "clustering data gives: " << clustering->size() << std::endl;

  // char statisticFN[256], resultFN[256];
  // setOutputNames( statisticFN, resultFN );
  // std::ofstream resultOut(resultFN), statOut(statisticFN);

  // statOut << "CAST clustering with parameters:  "
  //         << "Data input: " << progOpt.dataInFile  << ", "
  //         << "Label-Post input: " << progOpt.labPosInFile <<  ", "
  //         << "Max distance: " << progOpt.maxDistance <<  ", "
  //         << "CAST threshold: " << progOpt.thresCAST << std::endl << std::endl;  
  // statOut << "Obtains:  " << clustering->size() << " clusters. Takes: " << timer.display() << std::endl;
  
  // outputClustering(clustering, resultOut, labels.size());
  // resultOut.close(); statOut.close();

  // evaluateClustering( clustering, labelMap.size() );

  return 0;
}


/////////////////////////////////////////////////////////////////
ApplicationOptions getProgramOptions(int argc, char** argv)
{
  ApplicationOptions result;
  std::string appName = boost::filesystem::basename(argv[0]);
  po::options_description optDesc("Options");
      
  try  {
    /** Define and parse the program options 
     */
    optDesc.add_options()
        ("help,h", "Print help messages")
        ("dinput,i", po::value<std::string>(&result.dataInFile)->required(), "Data Input filename")
        ("lpinput", po::value<std::string>(&result.labPosInFile)->required(), "Label-Pos Input filename")        
        ("header,d", po::value<int>(&result.hasHeader)->default_value(0), "Indicates whether the "
         "input file has header row. false by default")
        ("maxdist,m", po::value<unsigned>(&result.maxDistance)->default_value(0), "max distance threshold of"
         "two consecutive SNPs to be considered") 
        ("simithres", po::value<double>(&result.thresSimMatrix)->default_value(0), "Similarity matrix threshold") 
  
        ("cast", po::value<double>(&result.thresCAST)->default_value(0.0), "CAST threshold")
        ;
    po::variables_map vm; 
    try { 
      po::store(po::command_line_parser(argc, argv).options(optDesc).run(), vm); // throws on error
      if (vm.count("help") ) {
        rad::OptionPrinter::printStandardAppDesc(appName,std::cout, optDesc, NULL);
        exit(1);
      }
      po::notify(vm);   	    

    } 
    catch(boost::program_options::required_option& e) /** missing arguments **/
    {
      rad::OptionPrinter::formatRequiredOptionError(e);
      std::cout << e.what() << std::endl << std::endl;
      rad::OptionPrinter::printStandardAppDesc(appName,std::cout,
                                               optDesc, NULL);
      exit(-1);
    }

  }
  catch(std::exception e)    
  {
    std::cout << "Unhandled Exception reached the top of main: "
              << e.what() << ", application will now exit" << std::endl;

    rad::OptionPrinter::printStandardAppDesc(appName, std::cout, optDesc, NULL);
    exit(-1);
  }

  return result;
}



Matrix* loadDataTable( const std::string& infile, const bool& hasHeader,
                      const char& sep, const char& quote)
{

  Matrix* dt = new Matrix();
  dt->reserve(100000);

  std::ifstream matrixFile(infile.c_str());
  if (!matrixFile) return dt;
  utl::CSVIterator<int> matrixLine(matrixFile);
  
  if (hasHeader)
    ++matrixLine;
  
  for( ; matrixLine != utl::CSVIterator<int>(); ++matrixLine ) {         
    std::vector<int> row(matrixLine->size(), 0);
    for (unsigned i = 0; i < matrixLine->size(); ++i) {
      row[i] = matrixLine->at(i);
    }
    dt->push_back(row);    
  }

  dt->resize(dt->size());
  return dt;
}


void loadLabelPosition( std::vector<Label> & labels,
                        std::vector<Position>& positions,
                        const std::string& infile ) { 
  std::ifstream labPosFile(infile.c_str());
  if (!labPosFile) return;
  std::vector<Label>().swap(labels); //lab2Pos.clear();
  std::vector<Position>().swap(positions); //.clear();
  utl::CSVIterator<std::string> labPosLine(labPosFile);// ++labPosLine;
  for( ; labPosLine != utl::CSVIterator<std::string>(); ++labPosLine ) {
    std::string label =  (*labPosLine)[1];
    int position = boost::lexical_cast<int>( (*labPosLine)[2]);
    labels.push_back(label);
    positions.push_back(position);
  }

  std::cout << "load " << labels.size() << " variables.\n";
}
