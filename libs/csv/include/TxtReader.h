/////////////////////////////////////////////////////////////////////////////
// Name:        TxtReader.h
// Project:     scLib
// Purpose:     Reader for text files, output: scDataNode
// Author:      Piotr Likus
// Modified by:
// Created:     01/12/2008
/////////////////////////////////////////////////////////////////////////////


#ifndef _TXTREADER_H__
#define _TXTREADER_H__

// ----------------------------------------------------------------------------
// Description
// ----------------------------------------------------------------------------
/// \file TxtReader.h
/// \brief Reader for text files, output: scDataNode
/// 

// ----------------------------------------------------------------------------
// Headers
// ----------------------------------------------------------------------------
#include <vector>
#include "sc/dtypes.h"

// ----------------------------------------------------------------------------
// Simple type definitions
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Forward class definitions
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Constants
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Class definitions
// ----------------------------------------------------------------------------
class scTxtReaderValueParser {
public:
  scTxtReaderValueParser() {}
  virtual ~scTxtReaderValueParser() {}
  virtual void parseValue(const scString &value, scDataNodeValueType valueType, scDataNode &output) = 0;
};

class scTxtReader {
public:
  scTxtReader();
  virtual ~scTxtReader();
  //returns <false> if parsing should be stopped
  virtual bool parseLine(const scString &line, scDataNode &output) = 0;
  void parseFile(const scString &fname, scDataNode &output);
  void parseText(const scString &input, scDataNode &output);
  void setParser(scTxtReaderValueParser *parser);
protected:
  virtual void initParse();
  virtual void initParseFile(const scString &fname);
  virtual void finalizeParseFile();
  virtual void initParseText(const scString &input);
  virtual void finalizeParseText();  
  virtual void finalizeParse();
  // returns <false> if there is no more data to read
  virtual bool readFileLine(scString &output);
  virtual bool readTextLine(scString &output);
  virtual void parseDataFromFile(scDataNode &output);
  virtual void parseDataFromText(scDataNode &output);
  virtual void handleParsedLine(scDataNode &input, scDataNode &output);
protected:
  void *m_fileHandle;
  scString *m_text; 
  size_t m_pos;
  scTxtReaderValueParser *m_parser;
};

#endif // _TXTREADER_H__
