
#include <string>
#include <boost/lexical_cast.hpp> 
#include <stdlib.h>     /* abs */

#include "MutInfo.hpp"
#include "CAST.hpp"

namespace fltm {
/////////////////////////////////////////////////////////////////////////

CAST_Partition* CAST::operator()( SimilarityCompute* simCompute,  const std::vector<unsigned>& local2Global) {
  CAST_Partition* result = new CAST_Partition();
  if (local2Global.empty())
    return result;
  CAST_Cluster unAssignedCluster;
  for (Index i = 0; i < local2Global.size(); ++i) { 
    unAssignedCluster.push_back(CAST_Item(i, local2Global.at(i), 0.0) );    
  }
  
  return performClustering( simCompute, unAssignedCluster);  
}

///////////////////////////////////////////////////////////////////////////////
CAST_Partition* CAST::performClustering( SimilarityCompute* simCompute, 
                                         CAST_Cluster& unAssignedCluster) {
  CAST_Partition* result = new CAST_Partition; // @todo: change the name of the alias

  while ( unAssignedCluster.size() ) { 
    CAST_Cluster* openCluster = new CAST_Cluster; 
    resetAffinity(unAssignedCluster);

    bool changesOccurred = true;
    while (changesOccurred && unAssignedCluster.size()) {     
      changesOccurred = false;
      AffinityCompute* maxCompute = new AffinityCompute();
      AffinityCompute* minCompute = new AffinityCompute();
      while ( unAssignedCluster.size() ) {
        Index maxAffIdx = (*maxCompute)(unAssignedCluster, std::greater<float>());
        if ( unAssignedCluster.at(maxAffIdx).affinity >= thresCAST*openCluster->size() ) {
          changesOccurred = true;
          addGoodItem( unAssignedCluster, *openCluster, simCompute, maxAffIdx );
        } else {
          break;
        }
      }
      delete maxCompute;
      while( unAssignedCluster.size() ) {      
        Index minAffIdx = (*minCompute)(*openCluster, std::less<float>());
        if ( openCluster->at(minAffIdx).affinity < thresCAST*openCluster->size() ) {
          changesOccurred = true;
          removeBadItem( unAssignedCluster, *openCluster, simCompute, minAffIdx );
        } else {
          break;
        }       
      }
      delete minCompute;
    }    
    openCluster->set_name(boost::lexical_cast<std::string>(result->size())); 
    result->insert( openCluster );

  }  
  return result;
  
}

void resetAffinity( CAST_Cluster& cluster) {
  for ( typename CAST_Cluster::iterator it = cluster.begin(); it != cluster.end(); ++it ) {
    it->affinity = 0.0;
  }
}

/** Searches for the optimal item in given cluster
 *
 */
template<typename Compare>
const Index AffinityCompute::operator()(const CAST_Cluster& clust, Compare comp)
{
  Index result = 0; 
  for (Index idx = 1; idx < clust.size(); idx++) {
    if ( comp(clust.at(idx).affinity, clust.at(result).affinity) ) { 
      result = idx;    
    }
  }
  
  return result;
}


//////////////////////////////////////////////////////////////////////////////////////
void addGoodItem( CAST_Cluster& unAssignedCluster, CAST_Cluster& openCluster, 
                  SimilarityCompute* simCompute, const Index clusterIdx ) {  
  updateClustersAffinity( unAssignedCluster, openCluster, simCompute,
                          clusterIdx );  
}

void removeBadItem( CAST_Cluster& unAssignedCluster, CAST_Cluster& openCluster, 
                    SimilarityCompute* simCompute, const Index clusterIdx ) {
  updateClustersAffinity( openCluster, unAssignedCluster, simCompute,
                          clusterIdx); 
}

void updateClustersAffinity( CAST_Cluster& sourceCluster, CAST_Cluster& targetCluster,
                             SimilarityCompute* simCompute, 
                             const Index clusterIndex )
{
  const CAST_Item item = sourceCluster.at(clusterIndex);
  moveItemBetweenClusters( sourceCluster, targetCluster, clusterIndex );
  for (Index i = 0; i < sourceCluster.size(); i++) {
    sourceCluster[i].affinity += (*simCompute)( sourceCluster[i].matrixIndex, item.matrixIndex);
    //getSimilarity( simComp, sourceCluster[i].matrixIndex, item.matrixIndex );
  }

  for (Index i = 0; i < targetCluster.size(); i++) {
    targetCluster[i].affinity += (*simCompute)( targetCluster[i].matrixIndex, item.matrixIndex);
    //getSimilarity( simMatrix, targetCluster[i].matrixIndex, item.matrixIndex );
  } 

}

void moveItemBetweenClusters( CAST_Cluster& source, CAST_Cluster& target, const Index clusterIndex ) {
  target.push_back( source.at(clusterIndex) );
  source.remove(clusterIndex);
}


// /**
//  *
//  */
// double getSimilarity( SimiMatrix& simMat, const Index& varA, const Index& varB ) {
//   if (varA == varB) return 1.0;
//   return (varA <= varB) ? simMat[varA][varB] : simMat[varB][varA]; 
// }


SimilarityCompute::SimilarityCompute( const Positions& pos ,
                                      Matrix* mat,
                                      float simiThres_,
                                      unsigned maxDist_ ):
    positions(pos), matrix(mat), simiThres(simiThres_), maxDist(maxDist_), entropyMap(std::vector<float> (pos.size(), -1.0))
{
  nbrVars = pos.size();
}


float SimilarityCompute::operator()( unsigned varA,
                                     unsigned varB )
{
  if (varA == varB) return 1.0;
  if ( abs( positions[varA] - positions[varB] ) > maxDist ) {
    return 0.0;
  }
  
  unsigned key = 2*nbrVars*varA + varB;
  SimiCache::iterator iter = simiCache.find(key);
  if ( iter == simiCache.end()) {
    Entropy<EMP> entropy; // empirical entropy (estimates the probability by frequency)
    JointEntropy<EMP> mutEntropy; // empirial mutual entropy;
    if ( entropyMap.at(varA) < 0.0 ) { 
      entropyMap[varA] = entropy( matrix->at(varA) ); // computes entropy of varA only if not already done
    }        
    if ( entropyMap.at(varB) < 0.0 ) { // computes entropy of varB only if not already done
      entropyMap[varB] = entropy( matrix->at(varB)) ; // since this operation could be expensive
    }
    
    double minEntropyAB = std::min( entropyMap[varA], entropyMap[varB]); // takes the min
    float result = 0.0;
    if (minEntropyAB != 0) {
      double mutEntropyAB = mutEntropy( matrix->at(varA), matrix->at(varB) );
      double mutualInfoAB = entropyMap[varA] + entropyMap[varB] - mutEntropyAB; // classic formula
      double normalizedMutInfo = mutualInfoAB / minEntropyAB;
      result = (normalizedMutInfo) ;// > simiThres) ? 1.0 : 0.0;
      simiCache[key] =  result;
    }
    return result;
  } else {
    return iter->second;
  }
  
}


}
