#ifndef __KSCRIPT_FUNC_H__
#define __KSCRIPT_FUNC_H__

#include "kscript_value.h"
#include "kscript_context.h"
#include "kscript_ptr.h"
#include "kscript_parsenode.h"

#include <qshared.h>

class KSParseNode;
class KSInterpreter;

KSModule::Ptr ksCreateModule_KScript( KSInterpreter* );

class KSFunction : public QShared
{
public:
  typedef KSSharedPtr<KSFunction> Ptr;

  KSFunction( KSModule* m ) : QShared(), m_module( m ) { }

  virtual bool call( KSContext& context ) = 0;
  virtual bool isSignal() const = 0;
  virtual QString name() const = 0;

  KSModule* module() { return m_module; }

private:
  KSModule* m_module;
};

typedef bool (*KSBuiltinFuncPtr)( KSContext& context );

class KSBuiltinFunction : public KSFunction
{
public:
  KSBuiltinFunction( KSModule* m, const QString& _name, KSBuiltinFuncPtr func ) : KSFunction( m ) { m_func = func; m_name = _name; }
  virtual ~KSBuiltinFunction() { }

  virtual bool call( KSContext& context ) { return m_func( context ); }
  virtual bool isSignal() const { return false; }
  virtual QString name() const { return m_name; }

private:
  KSBuiltinFuncPtr m_func;
  QString m_name;
};

class KSScriptFunction : public KSFunction
{
public:
  /**
   * The object does NOT take over the ownership of the module
   * since the module would never be deleted then. That is because
   * the module owns this function.
   */
  KSScriptFunction( KSModule* m, KSParseNode* node ) : KSFunction( m ) { m_node = node; }
  
  virtual bool call( KSContext& context );
  virtual bool isSignal() const;
  virtual QString name() const { return m_node->getIdent(); }
  
private:
  KSParseNode* m_node;
};

#endif
