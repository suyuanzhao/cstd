// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h
#ifndef __I_XML_READER_H_INCLUDED__
#define __I_XML_READER_H_INCLUDED__
//! An xml reader for wide characters, derived from IReferenceCounted.
/** This XML Parser can read any type of text files from any source
Irrlicht can read. Just call IFileSystem::createXMLReader(). For more
informations on how to use the parser, see IIrrXMLReader */
typedef IIrrXMLReader<char, IReferenceCounted> IXMLReader;
//! An xml reader for ASCII or UTF-8 characters, derived from IReferenceCounted.
/** This XML Parser can read any type of text files from any source
Irrlicht can read. Just call IFileSystem::createXMLReaderUTF8(). For
more informations on how to use the parser, see IIrrXMLReader */
typedef IIrrXMLReader<c8, IReferenceCounted> IXMLReaderUTF8;
#endif
