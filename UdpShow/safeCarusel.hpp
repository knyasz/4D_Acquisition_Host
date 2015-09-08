/* 
 * File:   safeCarusel.hpp
 * Author: AlexD
 *
 * Created on 20 אוגוסט 2015, 13:28
 */

#ifndef SAFECARUSEL_HPP
#define	SAFECARUSEL_HPP

//defines for simpler functions defines
#define TSafeCarTemplate template <typename TCrslEntry> 
#define TSafeCarusel     CSafeCarousel<TCrslEntry>

namespace NSafeContainer
{
    
    /////////////////////////////////////////////////////////////////////////////// 
    // cto'r
    /////////////////////////////////////////////////////////////////////////////// 
    TSafeCarTemplate TSafeCarusel::CSafeCarousel() :
            m_cellsCount(0),
            m_giveIndex(0),
            m_freeCellCount(0),
            m_createOK(false)
    {}
    
    ///////////////////////////////////////////////////////////////////////////////
    // dto'r
    /////////////////////////////////////////////////////////////////////////////// 
    TSafeCarTemplate TSafeCarusel::~CSafeCarousel()
    {}
    
    ///////////////////////////////////////////////////////////////////////////////
    // Function Name: create
    // Description:   Provides creation of safe carousel object
    // Output:        None
    // In:            cellsCount - number of cell to create
    // Return:        True/False
    ///////////////////////////////////////////////////////////////////////////////
    TSafeCarTemplate bool TSafeCarusel::create(unsigned int cellsCount)
    {
        bool rv(false);
        
        if (!m_createOK && cellsCount >0)
        {
            m_cellsCount = cellsCount;
            m_freeCellCount = cellsCount;
            
            if (nullptr == m_poolPtr)
            {
              m_poolPtr.reset(new(std::nothrow) SSafeEntry[m_cellsCount]);
            }
            
            if (m_poolPtr != nullptr)
            {
                m_createOK = true;
                rv = true;
            }
        }
        
        return rv;
    }
    
    ///////////////////////////////////////////////////////////////////////////////
    // Function Name: isCreated
    // Description:   retrives if safe carousel is allready created
    // Output:        None
    // In:            None
    // Return:        True/False
    ///////////////////////////////////////////////////////////////////////////////
    TSafeCarTemplate bool TSafeCarusel::isCreated()
    {
        return m_createOK;
    }
    
    ///////////////////////////////////////////////////////////////////////////////
    // Function Name: giveCell
    // Description:   Retrive next free cell
    // Output:        None
    // In:            None
    // Return:        Pointer to the cell
    ///////////////////////////////////////////////////////////////////////////////
    TSafeCarTemplate TCrslEntry* TSafeCarusel::giveCell(unsigned int& Key)
    {
       TCrslEntry* rv(nullptr);
       unsigned int ind(0);
       
     
       if (m_createOK)
       {
           std::lock_guard<std::mutex> lock(m_mutex);
           
           for (ind = 0; (ind < m_cellsCount) && (rv == nullptr); ind++)
           {
               m_giveIndex = (m_giveIndex + 1)%m_cellsCount;
               
               if (m_poolPtr[m_giveIndex].isFree)
               {
                   m_poolPtr[m_giveIndex].isFree = false;
                   rv = &(m_poolPtr[m_giveIndex].entry);
                   
                   Key = m_giveIndex;
                   
                  --m_freeCellCount;
               }
           }
       }
       
       return rv;
    }
    
    ///////////////////////////////////////////////////////////////////////////////
    // Function Name: getCellByKey
    // Description:   retrive existing cell by its key
    // Output:        None
    // In:            None
    // Return:        Pointer to the cell
    ///////////////////////////////////////////////////////////////////////////////
    TSafeCarTemplate TCrslEntry* TSafeCarusel::getCellByKey(const unsigned int Key)
    {
        TCrslEntry* rv(nullptr);
        
        if (m_createOK)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            
            if (Key < m_cellsCount)
            {
                if (!m_poolPtr[Key].isFree)
                {
                    rv = &(m_poolPtr[Key].entry);
                }
            }
        }
        
        return rv;
    }
    
    ///////////////////////////////////////////////////////////////////////////////
    // Function Name: releaseCell
    // Description:   release the cell
    // Output:        None
    // In:            Key - key to the cell to release
    // Return:        true/false
    ///////////////////////////////////////////////////////////////////////////////
    TSafeCarTemplate bool TSafeCarusel::releaseCell(const unsigned int Key)
    {
        bool rv(false);
        
        if (m_createOK)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            
            if (Key < m_cellsCount)
            {
                if (!m_poolPtr[Key].isFree)
                {
                    m_poolPtr[Key].isFree = true;
                    ++m_freeCellCount;
                    rv = true;
                }
            }
        }
        
        return rv;
    }
    
    ///////////////////////////////////////////////////////////////////////////////
    // Function Name: releaseCell
    // Description:   release the cell
    // Output:        None
    // In:            cell - cell to release
    // Return:        true/false
    ///////////////////////////////////////////////////////////////////////////////
    TSafeCarTemplate bool TSafeCarusel::releaseCell(const TCrslEntry* cell)
    {
        bool rv(false);
        
        const unsigned int ENTRY_SIZE(sizeof(SSafeEntry));
        unsigned long long cellOffset(0);
        
        if (m_createOK)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            
            if ((cell >= &(m_poolPtr[0].entry)) && (cell <= &(m_poolPtr[m_cellsCount - 1].entry)))
            {
                cellOffset = reinterpret_cast<unsigned char*>(cell) - reinterpret_cast<unsigned char*>(&(m_poolPtr[0].entry));
                
                //validity check 
                if (cellOffset % ENTRY_SIZE == 0)
                {
                    m_poolPtr[cellOffset / ENTRY_SIZE].isFree = true;
                    rv = true;
                    ++m_freeCellCount;
                }
            }
        }
        
        return rv;
    }
    
    
    ///////////////////////////////////////////////////////////////////////////////
    // Function Name: getTotalCellCount
    // Description:   get the number of cells
    // Output:        -
    // In:            -
    // Return:        number of cells
    ///////////////////////////////////////////////////////////////////////////////
    TSafeCarTemplate unsigned int TSafeCarusel::getTotalCellCount() const
    {
        return m_cellsCount;
    }
    
    ///////////////////////////////////////////////////////////////////////////////
    // Function Name: getFreeCellCount
    // Description:   get the number of free cells
    // Output:        -
    // In:            -
    // Return:        number of cells
    ///////////////////////////////////////////////////////////////////////////////
    TSafeCarTemplate unsigned int TSafeCarusel::getFreeCellCount() const
    {
        return m_freeCellCount;
    }
    
    ///////////////////////////////////////////////////////////////////////////////
    // Function Name: getUsedCellCount
    // Description:   get the number of active cells
    // Output:        -
    // In:            -
    // Return:        number of cells
    ///////////////////////////////////////////////////////////////////////////////
    TSafeCarTemplate unsigned int TSafeCarusel::getUsedCellCount() const
    {
        return m_cellsCount - m_freeCellCount;
    }
    
    
    ///////////////////////////////////////////////////////////////////////////////
    // Function Name: getUtilization
    // Description:   get carousel utilization
    // Output:        -
    // In:            -
    // Return:        utilization
    ///////////////////////////////////////////////////////////////////////////////
    TSafeCarTemplate unsigned int TSafeCarusel::getUtilization()
    {
        unsigned int rv(0);
        
        if (m_createOK)
        {
            rv = (getUtilization()/m_cellsCount) * 100;
        }
        
        return rv;
    }
    
    ///////////////////////////////////////////////////////////////////////////////
    // Function Name: getCellSize
    // Description:   get carousel cell size
    // Output:        -
    // In:            -
    // Return:        the cell size
    ///////////////////////////////////////////////////////////////////////////////
    TSafeCarTemplate unsigned int TSafeCarusel::getCellSize()
    {
       return sizeof(TCrslEntry); 
    }

}

#endif	/* SAFECARUSEL_HPP */

