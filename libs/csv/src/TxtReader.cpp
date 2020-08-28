/////////////////////////////////////////////////////////////////////////////
// Name:        TxtReader.h
// Project:     scLib
// Purpose:     Reader for text files, output: scDataNode
// Author:      Piotr Likus
// Modified by:
// Created:     01/12/2008
/////////////////////////////////////////////////////////////////////////////

//std
#include <fstream>

//sc
#include "sc/utils.h"
#include "sc/txtf/TxtReader.h"

#ifdef DEBUG_MEM
#include "sc/DebugMem.h"
#endif

scTxtReader::scTxtReader()
{
    m_parser = SC_NULL;
}

scTxtReader::~scTxtReader()
{
}

void scTxtReader::setParser(scTxtReaderValueParser *parser)
{
    m_parser = parser;
}

void scTxtReader::parseFile(const scString &fname, scDataNode &output)
{
  std::ifstream the_file( stringToStdString(fname) );
  m_fileHandle = &the_file;

  initParseFile(fname);
  try {
    initParse();
    try {    
      parseDataFromFile(output);
      finalizeParse();
    }
	  catch(...) {
	    finalizeParse();
	    throw;
	  }                
	  finalizeParseFile();
	}
	catch(...) {
	  finalizeParseFile();
	  throw;
	}          
}

void scTxtReader::parseText(const scString &input, scDataNode &output)
{
  initParseText(input);
  try {
    initParse();
    try {    
      parseDataFromText(output);
      finalizeParse();
    }
	  catch(...) {
	    finalizeParse();
	    throw;
	  }                  
	  finalizeParseText();
	}
	catch(...) {
	  finalizeParseText();
	  throw;
	}          
}

void scTxtReader::initParseFile(const scString &fname)
{
  std::ifstream *the_file = ((std::ifstream *)m_fileHandle);

  if (!the_file->is_open())
	  the_file->open(stringToStdString(fname));
  
  if (!the_file->is_open())
    throw scError("File open error: ["+fname+"]");
}

void scTxtReader::finalizeParseFile()
{
  std::ifstream *the_file = ((std::ifstream *)m_fileHandle);

  if (the_file)
  {
    the_file->close();
    m_fileHandle = SC_NULL;
  }  
}

void scTxtReader::initParseText(const scString &input)
{
  m_text = &const_cast<scString &>(input);
  m_pos = 0;
}

void scTxtReader::finalizeParseText()
{
  m_text = SC_NULL;
}

void scTxtReader::initParse()
{ //empty
}

void scTxtReader::finalizeParse()
{ //empty
}

bool scTxtReader::readFileLine(scString &output)
{
  std::string str;
  bool res;

  if (std::getline(*((std::ifstream *)m_fileHandle),str))
    res = true;
  else
    res = false;
      
  if (res)
    output = str.c_str();
  return res;
}

bool scTxtReader::readTextLine(scString &output)
{
  size_t m;
  size_t eolen;
  bool res = true;
  
  if (m_pos != scString::npos)
  {
    m = m_text->find_first_of("\n\r", m_pos);
    if (m != scString::npos)
    {
      eolen = 1;    
      if (
           (m_text->length()>m) 
           && 
           (((*m_text)[m+1] == '\n') || ((*m_text)[m+1] == '\r')) 
           && 
           ((*m_text)[m+1] != (*m_text)[m])
         ) 
      {
        eolen++;
      }       
      output = m_text->substr(m_pos, m - m_pos);
      m_pos = m + eolen;
    }
    else {
      output = m_text->substr(m_pos, m_text->length() - m_pos);
      m_pos = scString::npos;
    } 
  } else {  
    res = false;
  }
  return res;
}

void scTxtReader::parseDataFromFile(scDataNode &output)
{
 	scString line;
 	scDataNode item;
 	
 	while(readFileLine(line)) {
 	  if (!parseLine(line, item))
 	    break;
 	  handleParsedLine(item, output);  
  }
}

void scTxtReader::parseDataFromText(scDataNode &output)
{
 	scString line;
 	scDataNode item;
 	
 	while(readTextLine(line)) {
 	  if (!parseLine(line, item))
 	    break;
 	  handleParsedLine(item, output);  
  }
}

void scTxtReader::handleParsedLine(scDataNode &input, scDataNode &output)
{
   std::auto_ptr<scDataNode> newNodeGuard(new scDataNode());
   newNodeGuard.get()->eatValueFrom(input);
   output.addChild(newNodeGuard.release());
}
  
