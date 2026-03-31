// Test program to verify Daslang integration with JBMod
// This would normally be part of the game/engine, but we're making a standalone test

#include "daslang_vscript.h"
#include "cbase.h"
#include "tier0/memdbgon.h"

// Mock implementation of required interfaces for testing
class MockScriptManager : public IScriptManager
{
public:
    MockScriptManager() {}
    virtual ~MockScriptManager() {}
    
    virtual IScriptVM *CreateVM(ScriptLanguage_t language) override
    {
        if (language == SL_DASLANG)
        {
            return new CDaslangVM();
        }
        return nullptr;
    }
    
    virtual void DestroyVM(IScriptVM *pVM) override
    {
        delete pVM;
    }
    
    virtual const char *GetErrorMessage() override { return ""; }
    virtual bool Init() override { return true; }
    virtual void Shutdown() override {}
    virtual void *QueryInterface(const char *pInterfaceVersion) override { return nullptr; }
};

int main()
{
    printf("Testing Daslang integration with JBMod...\n");
    
    // Initialize tier0 (needed for memdbgon, etc.)
    // In a real scenario, this would be done by the engine
    
    // Create mock script manager
    MockScriptManager scriptManager;
    
    // Create Daslang VM
    IScriptVM *pVM = scriptManager.CreateVM(SL_DASLANG);
    if (!pVM)
    {
        printf("Failed to create Daslang VM\n");
        return 1;
    }
    
    printf("Successfully created Daslang VM\n");
    
    // Initialize the VM
    if (!pVM->Init())
    {
        printf("Failed to initialize Daslang VM\n");
        scriptManager.DestroyVM(pVM);
        return 1;
    }
    
    printf("Successfully initialized Daslang VM\n");
    
    // Test running a simple script
    const char *testScript = 
        "def add(a: int, b: int): int {\n"
        "    return a + b;\n"
        "}\n"
        "\n"
        "var result = add(5, 3);\n"
        "print(\"5 + 3 = \" + string(result));\n";
    
    ScriptStatus_t status = pVM->Run(testScript);
    if (status != SCRIPT_OK)
    {
        printf("Failed to run test script (status: %d)\n", status);
        pVM->Shutdown();
        scriptManager.DestroyVM(pVM);
        return 1;
    }
    
    printf("Successfully ran test script\n");
    
    // Cleanup
    pVM->Shutdown();
    scriptManager.DestroyVM(pVM);
    
    printf("Daslang integration test completed successfully!\n");
    return 0;
}