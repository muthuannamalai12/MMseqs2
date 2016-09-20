#include "DBReader.h"
#include "DBWriter.h"
#include "Debug.h"
#include "Util.h"
#include "PrefilteringIndexReader.h"
#include "Prefiltering.h"
#include "Parameters.h"

#include <iostream>     // std::ios, std::istream, std::cout
#include <fstream>      // std::ifstream
#include <vector>
#include <utility>      // std::pair

int createindex (int argc, const char **argv, const Command& command)
{
    Parameters& par = Parameters::getInstance();
    par.parseParameters(argc, argv, command, 1);
    if(par.split == 0){
        par.split = 1;
    }

#ifdef OPENMP
    omp_set_num_threads(par.threads);
#endif

    DBReader<unsigned int> dbr(par.db1.c_str(), par.db1Index.c_str());
    dbr.open(DBReader<unsigned int>::NOSORT);

    BaseMatrix* subMat = Prefiltering::getSubstitutionMatrix(par.scoringMatrixFile, par.alphabetSize, 8.0f, false);
    size_t kmerSize = par.kmerSize;
    if(par.kmerSize == 0){ // set k-mer based on aa size in database
        // if we have less than 10Mio * 335 amino acids use 6mers
        kmerSize = dbr.getAminoAcidDBSize() < 3350000000 ? 6 : 7;
    }
    Sequence seq(par.maxSeqLen, subMat->aa2int, subMat->int2aa, Sequence::AMINO_ACIDS, kmerSize, par.spacedKmer, par.compBiasCorrection);

    PrefilteringIndexReader::createIndexFile(par.db1, &dbr, &seq, subMat, par.split,
                                             subMat->alphabetSize, kmerSize, par.spacedKmer, par.diagonalScoring, par.threads);

    // write code
    dbr.close();
        
    delete subMat;
    return 0;
}