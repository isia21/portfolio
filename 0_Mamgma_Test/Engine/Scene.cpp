#include "stdafx.h"

#include "3DObject.h"
#include "Scene.h"

//-----------------------------------------------------------------------------
// Construction / Destruction
//-----------------------------------------------------------------------------
CScene::CScene(const char* pszName)
	: m_strName(pszName != nullptr ? pszName : "Untitled Scene")
	, m_strFilePath("")
	, m_bModified(false)
	, m_bStructureChanged(true)
{}

CScene::~CScene()
{
	Clear();
}

//-----------------------------------------------------------------------------
// Lifecycle
//-----------------------------------------------------------------------------
void CScene::Clear()
{
	for (C3DObject* pObject : m_vObjects)
	{
		SAFEDELETE(pObject);
	}
	m_vObjects.clear();

	m_strName = "Untitled Scene";
	m_strFilePath.clear();
	m_bModified = false;
	m_bStructureChanged = true;

	Utils::ODS("[SCENE] Cascade clear completed. Memory is clean.");
}

void CScene::CreateDefault()
{
	Clear();

	m_strName = "Default Test Scene";

	// --- 1. Ground plane ---
	{
		C3DObject* pObject = C3DObject::CreatePrimitive(C3DObject::ePR_Plane, 10.0f);
		if (pObject != nullptr)
		{
			pObject->SetPosition(0.0f, -2.0f, 0.0f);
			pObject->SetVisible(true);
			pObject->SetModelColor(0x808080FF);
			pObject->SetName("Ground Plane");
			AddObject(pObject);
		}
	}

	// --- 2. Red Cube ---
	{
		C3DObject* pObject = C3DObject::CreatePrimitive(C3DObject::ePR_Cube, 1.0f);
		if (pObject != nullptr)
		{
			pObject->SetPosition(-3.0f, 0.0f, 0.0f);
			pObject->SetVisible(true);
			pObject->SetModelColor(0xFF0000FF);
			pObject->SetName("Cube #1 (Red)");
			AddObject(pObject);
		}
	}

	// --- 3. Green Cube (Rotated) ---
	{
		C3DObject* pObject = C3DObject::CreatePrimitive(C3DObject::ePR_Cube, 1.5f);
		if (pObject != nullptr)
		{
			pObject->SetPosition(3.0f, 0.0f, 0.0f);
			pObject->SetRotation(0.0f, 45.0f, 0.0f);
			pObject->SetVisible(true);
			pObject->SetModelColor(0x00FF00FF);
			pObject->SetName("Cube #2 (Green)");
			AddObject(pObject);
		}
	}

	// --- 4. Yellow Sphere ---
	{
		C3DObject* pObject = C3DObject::CreatePrimitive(C3DObject::ePR_Sphere, 1.0f);
		if (pObject != nullptr)
		{
			pObject->SetPosition(-2.0f, 0.0f, 4.0f);
			pObject->SetVisible(true);
			pObject->SetModelColor(0xFFFF00FF);
			pObject->SetName("Sphere #1 (Yellow)");
			AddObject(pObject);
		}
	}

	// --- 5. Blue Sphere (Wireframe) ---
	{
		C3DObject* pObject = C3DObject::CreatePrimitive(C3DObject::ePR_Sphere, 1.5f);
		if (pObject != nullptr)
		{
			pObject->SetPosition(2.0f, 1.0f, 4.0f);
			pObject->SetRenderType( C3DObject::eRT_Wireframe);
			pObject->SetModelColor(0x0000FFFF);
			pObject->SetVisible(true);
			pObject->SetName("Sphere #2 (Blue Wire)");
			AddObject(pObject);
		}
	}

	// --- 6. Sine Plane (Сложный невыпуклый меш для нарезки) ---
	{
		C3DObject* pObject = C3DObject::CreatePrimitive(C3DObject::ePR_SinePlane, 10.0f);
		if (pObject != nullptr)
		{
			pObject->SetPosition(0.0f, 2.0f, -8.0f);
			pObject->SetModelColor(0xFF00FFFF);
			pObject->SetVisible(true);
			pObject->SetName("Sine Mesh (Source)");
			AddObject(pObject);
		}
	}

	m_bModified = false;

	Utils::ODS("[SCENE] Default scene '%s' created with %d objects.", m_strName.c_str(), static_cast<int>(m_vObjects.size()));
}

void CScene::Update(float fDeltaTime)
{
	// Анимации и обновления объектов сцены
	// (например, вращение ножа или покачивание волн)
}

//-----------------------------------------------------------------------------
// Object Management
//-----------------------------------------------------------------------------
void CScene::AddObject(C3DObject* pObject)
{
	if (pObject == nullptr)
		return;

	m_vObjects.push_back(pObject);
	m_bModified = true;
	m_bStructureChanged = true;
}

void CScene::RemoveObject(C3DObject* pObject)
{
	if (pObject == nullptr)
		return;

	for (auto it = m_vObjects.begin(); it != m_vObjects.end(); ++it)
	{
		if (*it == pObject)
		{
			delete* it;
			m_vObjects.erase(it);
			m_bModified = true;
			return;
		}
	}
}

//-----------------------------------------------------------------------------
// I/O and Serialization
//-----------------------------------------------------------------------------

static std::string SanitizeFileName(const std::string& str)
{
	std::string result = str;
	for (char& c : result)
	{
		if (c == ' ' || c == '#' || c == '(' || c == ')' || c == '/' || c == '\\')
			c = '_';
	}
	return result;
}

static Vector3 TransformVertexToWorld(const Vertex3D& v, const Vector3& pos, const Vector3& rot, const Vector3& scale)
{
	const float fDegToRad = 3.14159265358979323846f / 180.0f;

	// 1. Scale
	Vector3 res = { v.x * scale.x, v.y * scale.y, v.z * scale.z };

	// 2. Rotate Z
	if (rot.z != 0.0f)
	{
		const float radZ = rot.z * fDegToRad;
		const float cosZ = cosf(radZ);
		const float sinZ = sinf(radZ);
		const float rx = res.x * cosZ - res.y * sinZ;
		const float ry = res.x * sinZ + res.y * cosZ;
		res.x = rx;
		res.y = ry;
	}

	// 3. Rotate Y
	if (rot.y != 0.0f)
	{
		const float radY = rot.y * fDegToRad;
		const float cosY = cosf(radY);
		const float sinY = sinf(radY);
		const float rx = res.x * cosY + res.z * sinY;
		const float rz = -res.x * sinY + res.z * cosY;
		res.x = rx;
		res.z = rz;
	}

	// 4. Rotate X
	if (rot.x != 0.0f)
	{
		const float radX = rot.x * fDegToRad;
		const float cosX = cosf(radX);
		const float sinX = sinf(radX);
		const float ry = res.y * cosX - res.z * sinX;
		const float rz = res.y * sinX + res.z * cosX;
		res.y = ry;
		res.z = rz;
	}

	// 5. Translate
	res.x += pos.x;
	res.y += pos.y;
	res.z += pos.z;

	return res;
}

bool CScene::ExportToOBJ(const char* pszFilePath)
{
	if (pszFilePath == nullptr || strlen(pszFilePath) == 0)
	{
		Utils::ODS("[SCENE_ERROR] Cannot export: Target path is empty.");
		return false;
	}

	// --- create directory if it doesn't exist ---
	std::string sScenePath(pszFilePath);
	std::string sBasePath = "";
	size_t lastSlash = sScenePath.find_last_of("\\/");
	if (lastSlash != std::string::npos)
		sBasePath = sScenePath.substr(0, lastSlash + 1);

	std::string sModelsDir = sBasePath + "ModelsOBJ";
	CreateDirectoryA(sModelsDir.c_str(), NULL);

	// --- Open main Scene.obj for writing the assembly ---
	std::ofstream sceneFile(pszFilePath, std::ios::out | std::ios::trunc);
	if (!sceneFile.is_open())
	{
		Utils::ODS("[SCENE_ERROR] Failed to open file for OBJ export: %s", pszFilePath);
		return false;
	}

	sceneFile << "# Wavefront OBJ Scene Assembly Export\n";
	sceneFile << "# Generator: MeshCut Studio CAD Engine\n";
	sceneFile << "# Scene Name: " << m_strName << "\n\n";
	sceneFile << std::fixed << std::setprecision(6);

	size_t globalVertexOffset = 0; 

	for (size_t objIdx = 0; objIdx < m_vObjects.size(); ++objIdx)
	{
		C3DObject* pObj = m_vObjects[objIdx];
		if (pObj == nullptr || !pObj->IsVisible())
			continue;

		const std::string sCleanName = SanitizeFileName(pObj->GetName());

		// --- Create a unique filename for the individual object ---
		const std::string sSingleObjPath = sModelsDir + "/" + sCleanName + ".obj";
		pObj->ExportToOBJ(sSingleObjPath.c_str());

		// --- Write object to common Scene.obj with reference to its individual OBJ file ---
		sceneFile << "o " << sCleanName << "\n";

		const Vector3 pos = pObj->GetPosition();
		const Vector3 rot = pObj->GetRotation();
		const Vector3 scale = pObj->GetScale();

		const auto& vertices = pObj->GetVertices();
		for (const Vertex3D& v : vertices)
		{
			Vector3 worldPos = TransformVertexToWorld(v, pos, rot, scale);
			sceneFile << "v " << worldPos.x << " " << worldPos.y << " " << worldPos.z << "\n";
		}

		sceneFile << "\n";

		const auto& indices = pObj->GetIndices();
		for (size_t i = 0; i < indices.size(); i += 3)
		{
			if (i + 2 < indices.size())
			{
				sceneFile << "f "
					<< (indices[i] + 1 + globalVertexOffset) << " "
					<< (indices[i + 1] + 1 + globalVertexOffset) << " "
					<< (indices[i + 2] + 1 + globalVertexOffset) << "\n";
			}
		}

		sceneFile << "\n";
		globalVertexOffset += vertices.size();
	}

	sceneFile.close();
	Utils::ODS("[SCENE] Exported Scene Assembly to '%s' and individual models to '%s/'", pszFilePath, sModelsDir.c_str());
	return true;
}

bool CScene::LoadFromFile(const char* pszFilePath)
{
	if (pszFilePath == nullptr || strlen(pszFilePath) == 0)
		return false;

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(pszFilePath);
	if (!result)
	{
		Utils::ODS("[SCENE_ERROR] Failed to load Scene XML '%s': %s", pszFilePath, result.description());
		return false;
	}

	pugi::xml_node assembly = doc.child("CADAssembly");
	if (!assembly)
	{
		Utils::ODS("[SCENE_ERROR] <CADAssembly> root node missing in: %s", pszFilePath);
		return false;
	}

	// --- Clear existing scene objects before loading new ones ---
	Clear();

	m_strName = assembly.attribute("name").as_string("Loaded Scene");

	// --- Find Base Path ---
	std::string sScenePath(pszFilePath);
	std::string sBasePath = "";
	size_t lastSlash = sScenePath.find_last_of("\\/");
	if (lastSlash != std::string::npos)
		sBasePath = sScenePath.substr(0, lastSlash + 1);

	// --- Itterate through all component instances in the assembly ---
	pugi::xml_node instancesNode = assembly.child("ComponentInstances");
	for (pugi::xml_node inst : instancesNode.children("Instance"))
	{
		std::string sSource = inst.attribute("source").as_string();
		if (sSource.empty())
			continue;

		std::string sFullModelPath = sBasePath + sSource;

		// --- Create object and load its geometry from .model.xml ---
		C3DObject* pObj = new C3DObject();
		if (!pObj->LoadFromFile(sFullModelPath.c_str()))
		{
			Utils::ODS("[SCENE_WARN] Failed to load model file: %s", sFullModelPath.c_str());
			delete pObj;
			continue;
		}

		// --- Reset object name to the instance name in the assembly, if provided ---
		const char* pszInstName = inst.attribute("name").as_string(nullptr);
		if (pszInstName && strlen(pszInstName) > 0)
			pObj->SetName(pszInstName);

		// --- Apply Transform (Position, Rotation, Scale) ---
		pugi::xml_node transform = inst.child("Transform");
		if (transform)
		{
			pugi::xml_node pos = transform.child("Position");
			if (pos)
				pObj->SetPosition(pos.attribute("x").as_float(0.0f), pos.attribute("y").as_float(0.0f), pos.attribute("z").as_float(0.0f));

			pugi::xml_node rot = transform.child("Rotation");
			if (rot)
				pObj->SetRotation(rot.attribute("x").as_float(0.0f), rot.attribute("y").as_float(0.0f), rot.attribute("z").as_float(0.0f));

			pugi::xml_node scale = transform.child("Scale");
			if (scale)
				pObj->SetScale(scale.attribute("x").as_float(1.0f), scale.attribute("y").as_float(1.0f), scale.attribute("z").as_float(1.0f));
		}

		// --- Setup RenderState (Visibility, Render Type, Color) ---
		pugi::xml_node renderState = inst.child("RenderState");
		if (renderState)
		{
			pObj->SetVisible(renderState.attribute("visible").as_bool(true));

			std::string sMode = renderState.attribute("mode").as_string("Solid");
			pObj->SetRenderType((sMode == "Wireframe") ? C3DObject::eRT_Wireframe : C3DObject::eRT_Poligon);

			const char* pszColor = renderState.attribute("color").as_string("0xFFFFFFFF");
			pObj->SetModelColor( static_cast<unsigned int>(strtoul(pszColor, nullptr, 0)));
		}

		m_vObjects.push_back(pObj);
	}

	m_strFilePath = pszFilePath;
	m_bModified = false;
	m_bStructureChanged = true;

	Utils::ODS("[SCENE] Successfully loaded CAD Scene '%s' (%zu entities)", m_strName.c_str(), m_vObjects.size());
	return true;
}

bool CScene::SaveToFile(const char* pszFilePath)
{
	const char* pszTargetFile = (pszFilePath != nullptr && strlen(pszFilePath) > 0) ? pszFilePath : m_strFilePath.c_str();

	if (strlen(pszTargetFile) == 0)
	{
		Utils::ODS("[SCENE_ERROR] Cannot save: File path is empty.");
		return false;
	}

	std::string sScenePath(pszTargetFile);
	std::string sBasePath = "";
	size_t lastSlash = sScenePath.find_last_of("\\/");
	if (lastSlash != std::string::npos)
	{
		sBasePath = sScenePath.substr(0, lastSlash + 1);
	}

	std::string sModelsDir = sBasePath + "Models";
	CreateDirectoryA(sModelsDir.c_str(), NULL); 

	pugi::xml_document doc;

	pugi::xml_node decl = doc.prepend_child(pugi::node_declaration);
	decl.append_attribute("version") = "1.0";
	decl.append_attribute("encoding") = "UTF-8";

	pugi::xml_node assembly = doc.append_child("CADAssembly");
	assembly.append_attribute("version") = "1.0";
	assembly.append_attribute("name") = m_strName.c_str();

	pugi::xml_node meta = assembly.append_child("Metadata");
	meta.append_child("Generator").text().set("MeshCut Studio CAD Core");
	meta.append_child("Units").text().set("Millimeters");
	meta.append_child("UpAxis").text().set("Y");

	pugi::xml_node instancesNode = assembly.append_child("ComponentInstances");
	instancesNode.append_attribute("count") = static_cast<unsigned int>(m_vObjects.size());

	for (size_t i = 0; i < m_vObjects.size(); ++i)
	{
		C3DObject* pObj = m_vObjects[i];
		if (pObj == nullptr)
			continue;

		const std::string sCleanName = SanitizeFileName(pObj->GetName());
		const std::string sModelRelPath = "Models/" + sCleanName + ".model.xml";
		const std::string sModelFullPath = sBasePath + sModelRelPath;

		pObj->SaveToFile(sModelFullPath.c_str());

		pugi::xml_node inst = instancesNode.append_child("Instance");
		inst.append_attribute("id") = static_cast<unsigned int>(i);
		inst.append_attribute("name") = pObj->GetName();
		inst.append_attribute("source") = sModelRelPath.c_str();

		pugi::xml_node transform = inst.append_child("Transform");

		pugi::xml_node pos = transform.append_child("Position");
		pos.append_attribute("x") = pObj->GetPosition().x;
		pos.append_attribute("y") = pObj->GetPosition().y;
		pos.append_attribute("z") = pObj->GetPosition().z;

		pugi::xml_node rot = transform.append_child("Rotation");
		rot.append_attribute("x") = pObj->GetRotation().x;
		rot.append_attribute("y") = pObj->GetRotation().y;
		rot.append_attribute("z") = pObj->GetRotation().z;

		pugi::xml_node scale = transform.append_child("Scale");
		scale.append_attribute("x") = pObj->GetScale().x;
		scale.append_attribute("y") = pObj->GetScale().y;
		scale.append_attribute("z") = pObj->GetScale().z;

		pugi::xml_node renderState = inst.append_child("RenderState");
		renderState.append_attribute("visible") = pObj->IsVisible();
		renderState.append_attribute("mode") = (pObj->GetRenderType() == C3DObject::eRT_Wireframe) ? "Wireframe" : "Solid";

		char szHexColor[16] = {};
		sprintf_s(szHexColor, sizeof(szHexColor), "0x%08X", pObj->GetModelColor());
		renderState.append_attribute("color") = szHexColor;
	}

	const bool bSuccess = doc.save_file(pszTargetFile, "    ", pugi::format_default | pugi::format_indent);
	if (bSuccess)
	{
		m_strFilePath = pszTargetFile;
		m_bModified = false;
		Utils::ODS("[SCENE] Saved CAD Assembly and %zu Models to: %s", m_vObjects.size(), pszTargetFile);
	}
	else
	{
		Utils::ODS("[SCENE_ERROR] Failed to save Assembly XML: %s", pszTargetFile);
	}

	return bSuccess;
}

//-----------------------------------------------------------------------------
// Properties
//-----------------------------------------------------------------------------
void CScene::SetName(const char* pszName)
{
	m_strName = (pszName != nullptr) ? pszName : "Untitled Scene";
	m_bModified = true;
}

void CScene::SetFilePath(const char* pszFilePath)
{
	m_strFilePath = (pszFilePath != nullptr) ? pszFilePath : "";
}