/* 
 * File:   safeCarusel.h
 * Author: AlexD
 *
 * Created on 19 אוגוסט 2015, 13:28
 */

#ifndef SAFECARUSEL_H
#define	SAFECARUSEL_H

#include <mutex>
#include <memory>

namespace NSafeContainer
{

///////////////////////////////////////////////////////////////////////////////
// Template safe carousel that provides the user safe memory management capability
// the user get memory segment with knowlage that he is the only user of this segment
// the carusel has a reentrance protection , its by the user responsibility to release the carousel cell
// at the end of the use(give it back to the carousle pool)
/////////////////////////////////////////////////////////////////////////////// 
template <typename TCrslEntry>
class CSafeCarousel
{
    public:
    /////////////////////////////////////////////////////////////////////////////// 
    // cto'r
    /////////////////////////////////////////////////////////////////////////////// 
    CSafeCarousel();
    
    ///////////////////////////////////////////////////////////////////////////////
    // dto'r
    /////////////////////////////////////////////////////////////////////////////// 
    ~CSafeCarousel();
    
    ///////////////////////////////////////////////////////////////////////////////
    // Function Name: create
    // Description:   Provides creation of safe carousel object
    // Output:        None
    // In:            cellsCount - number of cell to create
    // Return:        True/False
    ///////////////////////////////////////////////////////////////////////////////
    bool create(unsigned int cellsCount);
    
    ///////////////////////////////////////////////////////////////////////////////
    // Function Name: isCreated
    // Description:   retrives if safe carousel is allready created
    // Output:        None
    // In:            None
    // Return:        True/False
    ///////////////////////////////////////////////////////////////////////////////
    bool isCreated();
    
    ///////////////////////////////////////////////////////////////////////////////
    // Function Name: giveCell
    // Description:   Retrive next free cell
    // Output:        None
    // In:            None
    // Return:        Pointer to the cell
    ///////////////////////////////////////////////////////////////////////////////
    TCrslEntry* giveCell(unsigned int& Key);
    
    ///////////////////////////////////////////////////////////////////////////////
    // Function Name: getCellByKey
    // Description:   retrive existing cell by its key
    // Output:        None
    // In:            None
    // Return:        Pointer to the cell
    ///////////////////////////////////////////////////////////////////////////////
    TCrslEntry* getCellByKey(const unsigned int Key);
    
    ///////////////////////////////////////////////////////////////////////////////
    // Function Name: releaseCell
    // Description:   release the cell
    // Output:        None
    // In:            Key - key to the cell to release
    // Return:        true/false
    ///////////////////////////////////////////////////////////////////////////////
    bool releaseCell(const unsigned int Key);
    
    ///////////////////////////////////////////////////////////////////////////////
    // Function Name: releaseCell
    // Description:   release the cell
    // Output:        None
    // In:            cell - cell to release
    // Return:        true/false
    ///////////////////////////////////////////////////////////////////////////////
    bool releaseCell(const TCrslEntry* cell);
    
    
    ///////////////////////////////////////////////////////////////////////////////
    // Function Name: getTotalCellCount
    // Description:   get the number of cells
    // Output:        -
    // In:            -
    // Return:        number of cells
    ///////////////////////////////////////////////////////////////////////////////
    unsigned int getTotalCellCount() const;
    
    ///////////////////////////////////////////////////////////////////////////////
    // Function Name: getFreeCellCount
    // Description:   get the number of free cells
    // Output:        -
    // In:            -
    // Return:        number of cells
    ///////////////////////////////////////////////////////////////////////////////
    unsigned int getFreeCellCount() const;
    
    ///////////////////////////////////////////////////////////////////////////////
    // Function Name: getUsedCellCount
    // Description:   get the number of active cells
    // Output:        -
    // In:            -
    // Return:        number of cells
    ///////////////////////////////////////////////////////////////////////////////
    unsigned int getUsedCellCount() const;
    
    
    ///////////////////////////////////////////////////////////////////////////////
    // Function Name: getUtilization
    // Description:   get carousel utilization
    // Output:        -
    // In:            -
    // Return:        utilization
    ///////////////////////////////////////////////////////////////////////////////
    unsigned int getUtilization();
    
    ///////////////////////////////////////////////////////////////////////////////
    // Function Name: getCellSize
    // Description:   get carousel cell size
    // Output:        -
    // In:            -
    // Return:        the cell size
    ///////////////////////////////////////////////////////////////////////////////
    unsigned int getCellSize();
    
    
    private:
        struct SSafeEntry
        {
          public:
            TCrslEntry entry;
            bool isFree;
            
            SSafeEntry() : isFree(true)
            {}
        };
        
    std::unique_ptr<SSafeEntry[]>  m_poolPtr;
    unsigned int                   m_cellsCount;
    unsigned int                   m_giveIndex;
    unsigned int                   m_freeCellCount;
    bool                           m_createOK;
    std::mutex                     m_mutex;
    
};


}

#include "safeCarusel.hpp"

#endif	/* SAFECARUSEL_H */

