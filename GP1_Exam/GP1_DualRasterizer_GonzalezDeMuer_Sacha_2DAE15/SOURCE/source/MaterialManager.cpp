#include "pch.h"
#include "MaterialManager.h"
#include "DXRenderer.h"

MaterialManager* MaterialManager::Get()
{
    static MaterialManager* m = new MaterialManager();
    return m;
}

MaterialManager::MaterialManager()
    :m_MaterialDictionary{}
{
}