



#ifndef LOWER

#ifndef CONJ
#ifdef XDOUBLE
#define KERNEL_FUNC xherk_kernel_UN
#elif defined(DOUBLE)
#define KERNEL_FUNC zherk_kernel_UN
#else
#define KERNEL_FUNC cherk_kernel_UN
#endif
#else
#ifdef XDOUBLE
#define KERNEL_FUNC xherk_kernel_UC
#elif defined(DOUBLE)
#define KERNEL_FUNC zherk_kernel_UC
#else
#define KERNEL_FUNC cherk_kernel_UC
#endif
#endif

#else

#ifndef CONJ
#ifdef XDOUBLE
#define KERNEL_FUNC xherk_kernel_LN
#elif defined(DOUBLE)
#define KERNEL_FUNC zherk_kernel_LN
#else
#define KERNEL_FUNC cherk_kernel_LN
#endif
#else
#ifdef XDOUBLE
#define KERNEL_FUNC xherk_kernel_LC
#elif defined(DOUBLE)
#define KERNEL_FUNC zherk_kernel_LC
#else
#define KERNEL_FUNC cherk_kernel_LC
#endif
#endif

#endif

#define KERNEL_OPERATION(M, N, K, ALPHA, SA, SB, C, LDC, X, Y) \
	KERNEL_FUNC(M, N, K, ALPHA[0], SA, SB, (FLOAT *)(C) + ((X) + (Y) * LDC) * COMPSIZE, LDC, (X) - (Y))

#if   !defined(LOWER) && !defined(TRANS)
#define SYRK_LOCAL    HERK_UN
#elif !defined(LOWER) &&  defined(TRANS)
#define SYRK_LOCAL    HERK_UC
#elif  defined(LOWER) && !defined(TRANS)
#define SYRK_LOCAL    HERK_LN
#else
#define SYRK_LOCAL    HERK_LC
#endif

#undef SCAL_K

#ifdef XDOUBLE
#define SCAL_K		QSCAL_K
#elif defined(DOUBLE)
#define SCAL_K		DSCAL_K
#else
#define SCAL_K		SSCAL_K
#endif


static inline int syrk_beta(BLASLONG m_from, BLASLONG m_to, BLASLONG n_from, BLASLONG n_to, FLOAT *alpha, FLOAT *c, BLASLONG ldc) {

  BLASLONG i;

#ifndef LOWER
  if (m_from > n_from) n_from = m_from;
  if (m_to   > n_to  ) m_to   = n_to;
#else
  if (m_from < n_from) m_from = n_from;
  if (m_to   < n_to  ) n_to   = m_to;
#endif

  c += (m_from + n_from * ldc) * COMPSIZE;

  m_to -= m_from;
  n_to -= n_from;

  for (i = 0; i < n_to; i++){

#ifndef LOWER

    SCAL_K(MIN(i + n_from - m_from + 1, m_to) * COMPSIZE, 0, 0, alpha[0], c, 1, NULL, 0, NULL, 0);

    if (i + n_from - m_from + 1 <= m_to)
      *(c + (i + n_from - m_from) * COMPSIZE + 1)  = ZERO;

    c += ldc * COMPSIZE;

#else

    SCAL_K(MIN(m_to - i + m_from - n_from, m_to) * COMPSIZE, 0, 0, alpha[0], c, 1, NULL, 0, NULL, 0);

    if (i < m_from - n_from) {
      c += ldc * COMPSIZE;
    } else {
      *(c + 1)  = ZERO;
      c += (1 + ldc) * COMPSIZE;
    }

#endif

  }

  return 0;
}

#ifdef THREADED_LEVEL3
#include "level3_syrk_threaded.c"
#else
#include "level3_syrk.c"
#endif
