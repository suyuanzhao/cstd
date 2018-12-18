
#include "Archive.hpp"

struct CUIntArray {
  
  DECLARE_DYNAMIC(CUIntArray);
  
  UINT* m_pData;   // the actual array of data
  int m_nSize;     // # of elements (upperBound - 1)
  int m_nMaxSize;  // MAX allocated
  int m_nGrowBy;   // grow amount
  
  
  
  // local typedefs for struct templates
  typedef UINT BASE_TYPE;
  typedef UINT BASE_ARG_TYPE;
  
  
  _AFXCOLL_INLINE int CUIntArray::GetSize() const
  { return m_nSize; }
  _AFXCOLL_INLINE int CUIntArray::GetUpperBound() const
  { return m_nSize-1; }
  _AFXCOLL_INLINE void CUIntArray::RemoveAll()
  { SetSize(0); }
  _AFXCOLL_INLINE UINT CUIntArray::GetAt(int nIndex) const
  { ASSERT(nIndex >= 0 && nIndex < m_nSize);
  return m_pData[nIndex]; }
  _AFXCOLL_INLINE void CUIntArray::SetAt(int nIndex, UINT newElement)
  { ASSERT(nIndex >= 0 && nIndex < m_nSize);
  m_pData[nIndex] = newElement; }
  
  _AFXCOLL_INLINE UINT& CUIntArray::ElementAt(int nIndex)
  { ASSERT(nIndex >= 0 && nIndex < m_nSize);
  return m_pData[nIndex]; }
  _AFXCOLL_INLINE const UINT* CUIntArray::GetData() const
  { return (const UINT*)m_pData; }
  _AFXCOLL_INLINE UINT* CUIntArray::GetData()
  { return (UINT*)m_pData; }
  _AFXCOLL_INLINE int CUIntArray::Add(UINT newElement)
  { int nIndex = m_nSize;
  SetAtGrow(nIndex, newElement);
  return nIndex; }
  
  _AFXCOLL_INLINE UINT CUIntArray::operator[](int nIndex) const
  { return GetAt(nIndex); }
  _AFXCOLL_INLINE UINT& CUIntArray::operator[](int nIndex)
  { return ElementAt(nIndex); }
  
  
  CUIntArray::CUIntArray()
  {
    m_pData = NULL;
    m_nSize = m_nMaxSize = m_nGrowBy = 0;
  }
  
  CUIntArray::~CUIntArray()
  {
    ASSERT_VALID(this);
    
    delete[] (BYTE*)m_pData;
  }
  
  void CUIntArray::SetSize(int nNewSize, int nGrowBy = -1)
  {
    ASSERT_VALID(this);
    ASSERT(nNewSize >= 0);
    
    if (nGrowBy != -1)
      m_nGrowBy = nGrowBy;  // set new size
    
    if (nNewSize == 0)
    {
      // shrink to nothing
      delete[] (BYTE*)m_pData;
      m_pData = NULL;
      m_nSize = m_nMaxSize = 0;
    }
    else if (m_pData == NULL)
    {
      // create one with exact size
#ifdef SIZE_T_MAX
      ASSERT(nNewSize <= SIZE_T_MAX/sizeof(UINT));    // no overflow
#endif
      m_pData = (UINT*) new BYTE[nNewSize * sizeof(UINT)];
      
      memset(m_pData, 0, nNewSize * sizeof(UINT));  // zero fill
      
      m_nSize = m_nMaxSize = nNewSize;
    }
    else if (nNewSize <= m_nMaxSize)
    {
      // it fits
      if (nNewSize > m_nSize)
      {
        // initialize the new elements
        
        memset(&m_pData[m_nSize], 0, (nNewSize-m_nSize) * sizeof(UINT));
        
      }
      
      m_nSize = nNewSize;
    }
    else
    {
      // otherwise, grow array
      int nGrowBy = m_nGrowBy;
      if (nGrowBy == 0)
      {
        // heuristically determine growth when nGrowBy == 0
        //  (this avoids heap fragmentation in many situations)
        nGrowBy = MIN(1024, MAX(4, m_nSize / 8));
      }
      int nNewMax;
      if (nNewSize < m_nMaxSize + nGrowBy)
        nNewMax = m_nMaxSize + nGrowBy;  // granularity
      else
        nNewMax = nNewSize;  // no slush
      
      ASSERT(nNewMax >= m_nMaxSize);  // no wrap around
#ifdef SIZE_T_MAX
      ASSERT(nNewMax <= SIZE_T_MAX/sizeof(UINT)); // no overflow
#endif
      UINT* pNewData = (UINT*) new BYTE[nNewMax * sizeof(UINT)];
      
      // copy new data from old
      memcpy(pNewData, m_pData, m_nSize * sizeof(UINT));
      
      // construct remaining elements
      ASSERT(nNewSize > m_nSize);
      
      memset(&pNewData[m_nSize], 0, (nNewSize-m_nSize) * sizeof(UINT));
      
      
      // get rid of old stuff (note: no destructors called)
      delete[] (BYTE*)m_pData;
      m_pData = pNewData;
      m_nSize = nNewSize;
      m_nMaxSize = nNewMax;
    }
  }
  
  int CUIntArray::Append(const CUIntArray& src)
  {
    ASSERT_VALID(this);
    ASSERT(this != &src);   // cannot append to itself
    
    int nOldSize = m_nSize;
    SetSize(m_nSize + src.m_nSize);
    
    memcpy(m_pData + nOldSize, src.m_pData, src.m_nSize * sizeof(UINT));
    
    return nOldSize;
  }
  
  void CUIntArray::Copy(const CUIntArray& src)
  {
    ASSERT_VALID(this);
    ASSERT(this != &src);   // cannot append to itself
    
    SetSize(src.m_nSize);
    
    memcpy(m_pData, src.m_pData, src.m_nSize * sizeof(UINT));
    
  }
  
  void CUIntArray::FreeExtra()
  {
    ASSERT_VALID(this);
    
    if (m_nSize != m_nMaxSize)
    {
      // shrink to desired size
#ifdef SIZE_T_MAX
      ASSERT(m_nSize <= SIZE_T_MAX/sizeof(UINT)); // no overflow
#endif
      UINT* pNewData = NULL;
      if (m_nSize != 0)
      {
        pNewData = (UINT*) new BYTE[m_nSize * sizeof(UINT)];
        // copy new data from old
        memcpy(pNewData, m_pData, m_nSize * sizeof(UINT));
      }
      
      // get rid of old stuff (note: no destructors called)
      delete[] (BYTE*)m_pData;
      m_pData = pNewData;
      m_nMaxSize = m_nSize;
    }
  }
  
  /////////////////////////////////////////////////////////////////////////////
  
  void CUIntArray::SetAtGrow(int nIndex, UINT newElement)
  {
    ASSERT_VALID(this);
    ASSERT(nIndex >= 0);
    
    if (nIndex >= m_nSize)
      SetSize(nIndex+1);
    m_pData[nIndex] = newElement;
  }
  
  
  
  
  
  void CUIntArray::InsertAt(int nIndex, UINT newElement, int nCount = 1)
  {
    
    ASSERT_VALID(this);
    ASSERT(nIndex >= 0);    // will expand to meet need
    ASSERT(nCount > 0);     // zero or negative size not allowed
    
    if (nIndex >= m_nSize)
    {
      // adding after the end of the array
      SetSize(nIndex + nCount);  // grow so nIndex is valid
    }
    else
    {
      // inserting in the middle of the array
      int nOldSize = m_nSize;
      SetSize(m_nSize + nCount);  // grow it to new size
      // shift old data up to fill gap
      memmove(&m_pData[nIndex+nCount], &m_pData[nIndex],
        (nOldSize-nIndex) * sizeof(UINT));
      
      // re-init slots we copied from
      
      memset(&m_pData[nIndex], 0, nCount * sizeof(UINT));
      
    }
    
    // insert new value in the gap
    ASSERT(nIndex + nCount <= m_nSize);
    
    
    
    // copy elements into the empty space
    while (nCount--)
      m_pData[nIndex++] = newElement;
    
  }
  
  
  
  void CUIntArray::RemoveAt(int nIndex, int nCount = 1)
  {
    ASSERT_VALID(this);
    ASSERT(nIndex >= 0);
    ASSERT(nCount >= 0);
    ASSERT(nIndex + nCount <= m_nSize);
    
    // just remove a range
    int nMoveCount = m_nSize - (nIndex + nCount);
    
    if (nMoveCount)
      memmove(&m_pData[nIndex], &m_pData[nIndex + nCount],
      nMoveCount * sizeof(UINT));
    m_nSize -= nCount;
  }
  
  void CUIntArray::InsertAt(int nStartIndex, CUIntArray* pNewArray)
  {
    ASSERT_VALID(this);
    ASSERT(pNewArray != NULL);
    ASSERT_KINDOF(CUIntArray, pNewArray);
    ASSERT_VALID(pNewArray);
    ASSERT(nStartIndex >= 0);
    
    if (pNewArray->GetSize() > 0)
    {
      InsertAt(nStartIndex, pNewArray->GetAt(0), pNewArray->GetSize());
      for (int i = 0; i < pNewArray->GetSize(); i++)
        SetAt(nStartIndex + i, pNewArray->GetAt(i));
    }
  }
  
  
  
  /////////////////////////////////////////////////////////////////////////////
  // Diagnostics
  
#ifdef _DEBUG
  void CUIntArray::Dump(CDumpContext& dc) const
  {
    
    
    dc << "with " << m_nSize << " elements";
    if (dc.GetDepth() > 0)
    {
      for (int i = 0; i < m_nSize; i++)
        dc << "\n\t[" << i << "] = " << m_pData[i];
    }
    
    dc << "\n";
  }
  
  void CUIntArray::AssertValid() const
  {
    
    
    if (m_pData == NULL)
    {
      ASSERT(m_nSize == 0);
      ASSERT(m_nMaxSize == 0);
    }
    else
    {
      ASSERT(m_nSize >= 0);
      ASSERT(m_nMaxSize >= 0);
      ASSERT(m_nSize <= m_nMaxSize);
      ASSERT(AfxIsValidAddress(m_pData, m_nMaxSize * sizeof(UINT)));
    }
  }
#endif //_DEBUG
  
};