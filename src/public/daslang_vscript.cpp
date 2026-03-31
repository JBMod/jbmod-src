//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Daslang VM implementation for VScript system
//=============================================================================//

#include "daslang_vscript.h"
#include "cbase.h"
#include "tier0/icommandline.h"
#include "tier0/memdbgon.h"
#include "vscript/ivscript.h"

// Include actual Daslang headers
#include <daScript/daScript.h>
#include <daScript/module/built_in.h>
#include <daScript/module/math.h>
#include <daScript/module/strings.h>
#include <daScript/module/rtti.h>
#include <daScript/module/debugger.h>
#include <daScript/module/jit.h>
#include <daScript/module/fio.h>
#include <daScript/module/network.h>
#include <dasModules/aotStandardLibrary.h> // from our cloned repo

// Forward declaration for our JBMod module registration function
extern void RegisterDaslangJBModModule(das::ModuleLibrary& lib);

CDaslangVM::CDaslangVM()
{
    m_pContext = nullptr;
    m_pModule = nullptr;
}

CDaslangVM::~CDaslangVM()
{
    Shutdown();
}

bool CDaslangVM::Init()
{
    // Initialize Daslang module system
    das::Module::Initialize();
    
    // Register all builtin modules
    das::register_builtin_modules();
    
    // Create module group for shared compilation state
    m_DummyLibGroup = das::ModuleGroup();
    
    // Create execution context
    m_pContext = new das::Context(4096); // 4K stack size
    
    if (!m_pContext)
    {
        Warning("Failed to create Daslang context\n");
        return false;
    }
    
    // Register JBMod-specific API bindings
    {
        das::ModuleLibrary lib(m_pContext);
        RegisterDaslangJBModModule(lib);
    }
    
    // Note: We don't create a persistent module here since we'll compile scripts on demand
    // The context is ready to evaluate compiled programs
    
    return true;
}

void CDaslangVM::Shutdown()
{
    if (m_pContext)
    {
        delete m_pContext;
        m_pContext = nullptr;
    }
    
    // Shutdown Daslang module system
    das::Module::Shutdown();
}

bool CDaslangVM::ConnectDebugger()
{
    // TODO: Implement debugger connection if needed
    return false;
}

void CDaslangVM::DisconnectDebugger()
{
    // TODO: Implement debugger disconnection if needed
}

ScriptLanguage_t CDaslangVM::GetLanguage()
{
    return SL_DASLANG;
}

const char *CDaslangVM::GetLanguageName()
{
    return "daslang";
}

void CDaslangVM::AddSearchPath(const char *pszSearchPath)
{
    // TODO: Implement search path functionality if needed
}

bool CDaslangVM::Frame(float simTime)
{
    // Daslang doesn't typically need per-frame processing
    return false;
}

// Script usage
ScriptStatus_t CDaslangVM::Run(const char *pszScript, bool bWait)
{
    if (!m_pContext || !pszScript)
        return SCRIPT_ERROR;
    
    // Compile the script
    das::CompileOptions options;
    das::ModuleGroup dummyLibGroup;
    auto program = das::compileDasScript(pszScript, options, dummyLibGroup);
    
    if (!program)
    {
        Warning("Failed to compile Daslang script: %s\n", pszScript);
        return SCRIPT_ERROR;
    }
    
    // Simulate (initialize globals)
    if (!program->simulate(*m_pContext))
    {
        Warning("Failed to simulate Daslang script: %s\n", pszScript);
        return SCRIPT_ERROR;
    }
    
    // Look for a main function to call
    das::Function* mainFunc = m_pContext->getFunction("main");
    if (!mainFunc)
    {
        // No main function, just return success (script executed for side effects)
        return SCRIPT_OK;
    }
    
    // Call the main function
    das::Context::FunctionCallContext callContext;
    if (!m_pContext->invoke(*mainFunc, callContext))
    {
        Warning("Failed to invoke main function in Daslang script: %s\n", pszScript);
        return SCRIPT_ERROR;
    }
    
    return SCRIPT_OK;
}

// Compilation
HSCRIPT CDaslangVM::CompileScript(const char *pszScript, const char *pszId)
{
    if (!pszScript)
        return nullptr;
    
    // Compile the script but don't simulate it yet
    das::CompileOptions options;
    das::ModuleGroup dummyLibGroup;
    auto program = das::compileDasScript(pszScript, options, dummyLibGroup);
    
    if (!program)
    {
        Warning("Failed to compile Daslang script: %s\n", pszScript);
        return nullptr;
    }
    
    // In a real implementation, we would wrap this in our HSCRIPT type
    // For now, we'll just return the program pointer cast to HSCRIPT
    // NOTE: This is a simplification - a real implementation would need proper reference counting
    return reinterpret_cast<HSCRIPT>(program.get());
}

void CDaslangVM::ReleaseScript(HSCRIPT hScript)
{
    // In a real implementation, we would properly release the reference
    // For our simplified approach, we do nothing as the unique_ptr will clean up
    // when it goes out of scope
}

// Execution of compiled
ScriptStatus_t CDaslangVM::Run(HSCRIPT hScript, HSCRIPT hScope, bool bWait)
{
    if (!m_pContext || !hScript)
        return SCRIPT_ERROR;
    
    // Retrieve the program from our HSCRIPT
    auto program = reinterpret_cast<std::unique_ptr<das::Program>*>(hScript);
    if (!program || !(*program))
        return SCRIPT_ERROR;
    
    // Simulate (initialize globals) if not already done
    if (!(*program)->simulate(*m_pContext))
    {
        Warning("Failed to simulate Daslang script\n");
        return SCRIPT_ERROR;
    }
    
    // Look for a main function to call
    das::Function* mainFunc = m_pContext->getFunction("main");
    if (!mainFunc)
    {
        // No main function, just return success (script executed for side effects)
        return SCRIPT_OK;
    }
    
    // Call the main function
    das::Context::FunctionCallContext callContext;
    if (!m_pContext->invoke(*mainFunc, callContext))
    {
        Warning("Failed to invoke main function in Daslang script\n");
        return SCRIPT_ERROR;
    }
    
    return SCRIPT_OK;
}

ScriptStatus_t CDaslangVM::Run(HSCRIPT hScript, bool bWait)
{
    return Run(hScript, NULL, bWait);
}

// Scope
HSCRIPT CDaslangVM::CreateScope(const char *pszScope, HSCRIPT hParent)
{
    // TODO: Implement proper scoping if needed
    // For now, we'll just return NULL as we don't implement scoping
    return nullptr;
}

HSCRIPT CDaslangVM::ReferenceScope(HSCRIPT hScript)
{
    // TODO: Implement proper scoping if needed
    // For now, we'll just return NULL as we don't implement scoping
    return nullptr;
}

void CDaslangVM::ReleaseScope(HSCRIPT hScript)
{
    // TODO: Implement proper scoping cleanup if needed
}

// Script functions
HSCRIPT CDaslangVM::LookupFunction(const char *pszFunction, HSCRIPT hScope, bool bNoDelegation)
{
    if (!m_pContext || !pszFunction)
        return nullptr;
    
    // Look up the function in our context
    das::Function* func = m_pContext->getFunction(pszFunction);
    if (!func)
        return nullptr;
    
    // In a real implementation, we would wrap this in our HSCRIPT type
    // For now, we'll just return the function pointer cast to HSCRIPT
    // NOTE: This is a simplification - a real implementation would need proper reference counting
    return reinterpret_cast<HSCRIPT>(func);
}

void CDaslangVM::ReleaseFunction(HSCRIPT hScript)
{
    // In a real implementation, we would properly release the reference
    // For our simplified approach, we do nothing
}

// External functions
void CDaslangVM::RegisterFunction(ScriptFunctionBinding_t *pScriptFunction)
{
    // TODO: Implement external function registration if needed
    // This would require integration with Daslang's extern function binding system
}

// External classes
bool CDaslangVM::RegisterClass(ScriptClassDesc_t *pClassDesc)
{
    // TODO: Implement external class registration if needed
    // This would require integration with Daslang's type binding system
    return false;
}

void CDaslangVM::RegisterAllClasses()
{
    // TODO: Implement external class registration if needed
}

// External instances
HSCRIPT CDaslangVM::RegisterInstance(ScriptClassDesc_t *pDesc, void *pInstance)
{
    // TODO: Implement external instance registration if needed
    // This would require integration with Daslang's type binding system
    return nullptr;
}

void CDaslangVM::SetInstanceUniqeId(HSCRIPT hInstance, const char *pszId)
{
    // TODO: Implement instance ID setting if needed
}

void *CDaslangVM::GetInstanceValue(HSCRIPT hInstance, ScriptClassDesc_t *pExpectedType)
{
    // TODO: Implement instance value retrieval if needed
    return nullptr;
}

// Value helpers
bool CDaslangVM::GenerateUniqueKey(const char *pszRoot, char *pBuf, int nBufSize)
{
    // TODO: Implement unique key generation if needed
    return false;
}

bool CDaslangVM::ValueExists(HSCRIPT hScope, const char *pszKey)
{
    // TODO: Implement value existence check if needed
    return false;
}

bool CDaslangVM::SetValue(HSCRIPT hScope, const char *pszKey, const char *pszValue)
{
    // TODO: Implement value setting if needed
    return false;
}

bool CDaslangVM::SetValue(HSCRIPT hScope, const char *pszKey, const ScriptVariant_t &value)
{
    // TODO: Implement value setting if needed
    return false;
}

bool CDaslangVM::SetValue(const char *pszKey, const ScriptVariant_t &value)
{
    // TODO: Implement value setting if needed
    return false;
}

void CDaslangVM::CreateTable(ScriptVariant_t &Table)
{
    // TODO: Implement table creation if needed
}

int CDaslangVM::GetNumTableEntries(HSCRIPT hScope)
{
    // TODO: Implement table entry counting if needed
    return 0;
}

int CDaslangVM::GetKeyValue(HSCRIPT hScope, int nIterator, ScriptVariant_t *pKey, ScriptVariant_t *pValue)
{
    // TODO: Implement key/value retrieval if needed
    return 0;
}

bool CDaslangVM::GetValue(HSCRIPT hScope, const char *pszKey, ScriptVariant_t *pValue)
{
    // TODO: Implement value retrieval if needed
    return false;
}

bool CDaslangVM::GetValue(const char *pszKey, ScriptVariant_t *pValue)
{
    // TODO: Implement value retrieval if needed
    return false;
}

void CDaslangVM::ReleaseValue(ScriptVariant_t &value)
{
    // TODO: Implement value release if needed
}

bool CDaslangVM::ClearValue(HSCRIPT hScope, const char *pszKey)
{
    // TODO: Implement value clearing if needed
    return false;
}

bool CDaslangVM::ClearValue(const char *pszKey)
{
    // TODO: Implement value clearing if needed
    return false;
}

void CDaslangVM::WriteState(CUtlBuffer *pBuffer)
{
    // TODO: Implement state writing if needed
}

void CDaslangVM::ReadState(CUtlBuffer *pBuffer)
{
    // TODO: Implement state reading if needed
}

void CDaslangVM::RemoveOrphanInstances()
{
    // TODO: Implement orphan instance removal if needed
}

void CDaslangVM::DumpState()
{
    // TODO: Implement state dumping if needed
}

void CDaslangVM::SetOutputCallback(ScriptOutputFunc_t pFunc)
{
    // TODO: Implement output callback if needed
}

void CDaslangVM::SetErrorCallback(ScriptErrorFunc_t pFunc)
{
    // TODO: Implement error callback if needed
}

bool CDaslangVM::RaiseException(const char *pszExceptionText)
{
    // TODO: Implement exception raising if needed
    return false;
}

// Helper methods
ScriptStatus_t CDaslangVM::ExecuteFunction(HSCRIPT hFunction, ScriptVariant_t *pArgs, int nArgs, ScriptVariant_t *pReturn, HSCRIPT hScope, bool bWait)
{
    if (!m_pContext || !hFunction)
        return SCRIPT_ERROR;
    
    // Retrieve the function from our HSCRIPT
    das::Function* func = reinterpret_cast<das::Function*>(hFunction);
    if (!func)
        return SCRIPT_ERROR;
    
    // TODO: Implement proper argument passing and return value handling
    // This would require converting ScriptVariant_t to Daslang values and back
    
    // For now, we'll just try to invoke the function with no arguments
    das::Context::FunctionCallContext callContext;
    if (!m_pContext->invoke(*func, callContext))
    {
        Warning("Failed to invoke Daslang function\n");
        return SCRIPT_ERROR;
    }
    
    return SCRIPT_OK;
}