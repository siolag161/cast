
#include "TimerUtils.hpp"
#include "ApplicationOptions.hpp"
#include "OptionPrinter.hpp" //for printing
#include "DataTable.hpp"
#include "CSVParser.hpp"
#include "DataTransformation.hpp"
#include "Range.hpp"

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
using namespace fltm;     

typedef utility::Range<float, utility::ITER_TYPE_ADD> FloatRange;
typedef utility::Range<unsigned, utility::ITER_TYPE_ADD> IntRange;

CAST::partition_ptr performClustering( const Matrix& matrix,
                                       const std::vector<Position> positions,
                                       const Clust_CAST_Opt& opt );

float evaluateClustering( const CAST_Partition& clustering);
void saveResults( ApplicationOptions& progOps,
                  Clust_CAST_Opt& opt,
                  const CAST_Partition& clustering,
                  const unsigned currentSize,
                  const float score,
                  const double elapsedTime );


int main(int argc, char** argv) {
  
  ApplicationOptions progOpt = getProgramOptions(argc, argv);

  utl::Timer timer; timer.start();
  std::vector<Label> labels; std::vector<Position> positions;
  std::cout << "loading data from " <<  progOpt.dataInFile << std::endl; // todo: logging
  std::shared_ptr<Matrix> matrix = loadDataTable ( progOpt.dataInFile );
  loadLabelPosition( labels, positions, progOpt.labPosInFile );
  std::cout << "data loaded. rows: " << utl::nrow(*matrix) << ", columns: "
            << utl::ncol(*matrix) << ". takes: " <<  timer.display() << std::endl << std::endl; // todo: logging
  timer.restart();

  FloatRange rangeCAST( progOpt.CAST_min, progOpt.CAST_max, progOpt.CAST_step );
  FloatRange rangeSimi( progOpt.simi_min, progOpt.simi_max, progOpt.simi_step );
  IntRange rangeMaxDist( progOpt.maxDist_min, progOpt.maxDist_max, progOpt.maxDist_step );

  std::vector<float> scores;
  std::vector<Clust_CAST_Opt> opts;
  
  for ( auto maxDist: rangeMaxDist )
    for ( auto simi: rangeSimi ) 
      for ( auto cast: rangeCAST ) {
        Clust_CAST_Opt option(maxDist, simi, cast);
        auto clustering = performClustering( *matrix, positions, option );
        auto score = evaluateClustering( *clustering );
        scores.push_back(score); opts.push_back(option);
        saveResults( progOpt, option, *clustering, positions.size(), score, timer.getElapsedTimeInSec() );
        timer.restart();
      }
  
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
        ("outputDir", po::value<std::string>(&result.outputDir)->required(), "Output Directory")

        ("minCast", po::value<float>(&result.CAST_min)->required(), "Min CAST")
        ("maxCast", po::value<float>(&result.CAST_max)->required(), "Max CAST")
        ("stepCast", po::value<float>(&result.CAST_step)->required(), "Step CAST")

        ("minSimi", po::value<float>(&result.simi_min)->required(), "Min Similarity")
        ("maxSimi", po::value<float>(&result.simi_max)->required(), "Max Similarity")
        ("stepSimi", po::value<float>(&result.simi_step)->required(), "Step Similarity")

        ("minMaxDist", po::value<unsigned>(&result.maxDist_min)->required(), "Min - Max Distance ")
        ("maxMaxDist", po::value<unsigned>(&result.maxDist_max)->required(), "Max - Max Distance")
        ("stepMaxDist", po::value<unsigned>(&result.maxDist_step)->required(), "Step - Max Distance")

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
      rad::OptionPrinter::printStandardAppDesc( appName,std::cout,
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



std::shared_ptr<Matrix> loadDataTable( const std::string& infile, const char& sep, const char& quote)
{

  std::shared_ptr<Matrix> dt( new Matrix() );
  dt->reserve(100000);
  std::ifstream matrixFile(infile.c_str());
  if (!matrixFile) return dt;
  
  utl::CSVIterator<int> matrixLine(matrixFile);
  
  for( ; matrixLine != utl::CSVIterator<int>(); ++matrixLine ) {         
    std::vector<int> row(matrixLine->size(), 0);
    for (unsigned i = 0; i < matrixLine->size(); ++i) {
      row[i] = matrixLine->at(i);
    }
    dt->push_back(row);    
  }

  //dt->resize(dt->size());
  std::cout << "done loading data" << std::endl << std::endl;
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

////////////////////////////////////////////////////////
CAST::partition_ptr performClustering( const Matrix& matrix,
                                       const std::vector<Position> positions,
                                       const Clust_CAST_Opt& opt ) {
  char opts[256];
  sprintf(opts, "simi: %.2f, maxDist: %d, cast: %.2f", opt.simi, opt.maxDist, opt.cast);
  std::cout << " performing clustering - " << opts << std::endl;
  SimilarityCompute simiCompute( positions, matrix, opt.simi, opt.maxDist );  
  CAST clusteringCAST( opt.cast ) ;
  std::vector<unsigned> labelMap; for (unsigned i = 0; i < positions.size(); ++i) labelMap.push_back(i);

  CAST::partition_ptr clustering = clusteringCAST( simiCompute, labelMap );
  std::cout << "nbr clusters obtained = " << clustering->size() << std::endl << std::endl;
  
  return clustering;
}

float evaluateClustering( const CAST_Partition& clustering) {
  return 0.0;
}

void saveResults( ApplicationOptions& progOpt,
                  Clust_CAST_Opt& opt,
                  const CAST_Partition& clustering,
                  const unsigned currentSize,
                  const float score,
                  const double elapsedTime )
{
    
  boost::filesystem::path outputPath = boost::filesystem::absolute(progOpt.outputDir);

  char cast_clustering_fn[256], cast_statistics_fn[256];
  char optBuf[80]; char timeBuf[80];

  time_t now = time(0); struct tm tstruct;
  tstruct = *localtime(&now);
  
  strftime(timeBuf, sizeof(timeBuf), "%Y_%m_%d_%H_%M_%S", &tstruct);
  sprintf( optBuf, "%.2f_%d_%.2f", opt.simi, opt.maxDist, opt.cast );  

  sprintf( cast_statistics_fn, "CAST_statistics_%s.csv", optBuf );  
  sprintf( cast_clustering_fn, "CAST_clustering_%s.csv", optBuf );

  outputPath /= timeBuf;
  boost::filesystem::create_directories(outputPath);
  saveClustering( clustering, currentSize, (outputPath / cast_clustering_fn).string() ) ;
  saveStatistics( clustering, score, elapsedTime, (outputPath / cast_statistics_fn).string() );
}


////////////////////////////////////////////////////////////////////
static const std::string ID = "id"; static const std::string LABEL = "label";
static const std::string LATENT = "latent"; static const std::string PARENT = "parent";
static const std::string LEVEL = "level"; static const std::string POSITION = "position";
static const std::string CARDINALITY = "cardinality";
static const std::string PARENT_ID = "parent_id";
static const char SEPARATOR = ',';

void saveClustering( const CAST_Partition& clustering, const unsigned currentSize, std::string clustFN )  {
  size_t currentLatentIndex = currentSize;
  std::ofstream clustOut;
  clustOut << ID << SEPARATOR << LATENT << SEPARATOR << PARENT << SEPARATOR
           << LEVEL << SEPARATOR << CARDINALITY << "\n";  // writes header

  std::cout << "saving clustering with " << currentSize << " into " << clustFN << std::endl;
  for ( auto iter = clustering.begin(); iter != clustering.end(); ++iter ) {
    for ( auto citer = iter->second->begin(); citer != iter->second->end(); ++citer ) {
      clustOut << citer->globalIndex << SEPARATOR << 0 << SEPARATOR <<  currentLatentIndex << SEPARATOR
               << 0 << SEPARATOR << 2 << std::endl;  // writes header
    }
    ++currentLatentIndex;
  }

  for (size_t latentId = currentSize; latentId < currentLatentIndex; ++latentId) {
    clustOut << latentId << SEPARATOR << 1 << SEPARATOR << -1 << SEPARATOR
             << 1 << SEPARATOR << 2 << std::endl;  // writes header
  }

  clustOut.close();
}

void saveStatistics( const CAST_Partition& clustering, const float score, const float elapsedTime, std::string statisticFN ) {
  std::cout << "saving statistics into: " << statisticFN << std::endl;

  std::ofstream statOut(statisticFN);
  std::cout << "CAST clustering gives: 0 " << clustering.size() << " clusters. "
            << "Scores: " << score << ". "
            << "Takes: " << utility::timeDisplay(elapsedTime) << std::endl;
  statOut.close();
}
