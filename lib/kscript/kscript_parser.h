#ifndef __PARSER_H__
#define __PARSER_H__

#include <stdio.h>
#include <qstring.h>

class KSParser;
class KSParseNode;
class KSContext;

extern KSParser *theParser;  // Defined in parser.cc
extern int yyparse();      // Defined through yacc.y

class KSParser 
{
public:
  KSParser( FILE* inp_file, const char *filename = NULL );
  ~KSParser();

  bool eval( KSContext& );

  bool parse();
  void setRootNode( KSParseNode *node );
  KSParseNode *getRootNode();
  void parse_error( const char *file, const char *err, int line );

  void print( bool detailed = false );

  KSParseNode* donateParseTree();

  QString errorMessage() const { return m_errorMessage; }

private:
  KSParseNode *rootNode;
  QString m_errorMessage;
};


#endif
