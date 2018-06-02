
for (j = 0; j < size.h; ++j)
{
  const arrtype* src = (const arrtype*)(((const uchar*)src0) + j * srcstep);
  arrtype* dst = (arrtype*)(((uchar*)dst0) + j * dststep);

  for (i = 0; i < (len + 1) / 2; i++) {
    arrtype t0 = src[i];
    arrtype t1 = src[len - i - 1];
    dst[i] = t1;
    dst[len - i - 1] = t0;
  }
}