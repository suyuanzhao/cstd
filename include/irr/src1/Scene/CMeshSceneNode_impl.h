// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h
//#include "CMeshSceneNode.h"
//#include "IVideoDriver.h"
//#include "ISceneManager.h"
//#include "ICameraSceneNode.h"
//#include "IMeshCache.h"
//#include "IAnimatedMesh.h"
//#include "IMaterialRenderer.h"
//#include "IFileSystem.h"
//#include "CShadowVolumeSceneNode.h"
//! constructor
CMeshSceneNode::CMeshSceneNode(IMesh* mesh, ISceneNode* parent, ISceneManager* mgr, s32 id,
    const float3 position, const float3 rotation,
    const float3 scale)
  : IMeshSceneNode(parent, mgr, id, position, rotation, scale), Mesh(0), Shadow(0),
    PassCount(0), ReadOnlyMaterials(false)
{
#ifdef _DEBUG
  setDebugName("CMeshSceneNode");
#endif
  setMesh(mesh);
}
//! destructor
CMeshSceneNode::~CMeshSceneNode()
{
  if (Shadow) {
    Shadow->drop();
  }
  if (Mesh) {
    Mesh->drop();
  }
}
//! frame
void CMeshSceneNode::OnRegisterSceneNode()
{
  if (IsVisible) {
    // because this node supports rendering of mixed mode meshes consisting of
    // transparent and solid material at the same time, we need to go through all
    // materials, check of what type they are and register this node for the right
    // render pass according to that.
    IVideoDriver* driver = SceneManager->getVideoDriver();
    PassCount = 0;
    int transparentCount = 0;
    int solidCount = 0;
    // count transparent and solid materials in this scene node
    if (ReadOnlyMaterials && Mesh) {
      // count mesh materials
      int i;
      for (i = 0; i < Mesh->getMeshBufferCount(); ++i) {
        IMeshBuffer* mb = Mesh->getMeshBuffer(i);
        IMaterialRenderer* rnd = mb ? driver->getMaterialRenderer(mb->getMaterial().MaterialType) : 0;
        if (rnd && rnd->isTransparent()) {
          ++transparentCount;
        }
        else {
          ++solidCount;
        }
        if (solidCount && transparentCount) {
          break;
        }
      }
    }
    else {
      // count copied materials
      int i;
      for (i = 0; i < Materials_size; ++i) {
        IMaterialRenderer* rnd =
            driver->getMaterialRenderer(Materials[i].MaterialType);
        if (rnd && rnd->isTransparent()) {
          ++transparentCount;
        }
        else {
          ++solidCount;
        }
        if (solidCount && transparentCount) {
          break;
        }
      }
    }
    // register according to material types counted
    if (solidCount) {
      SceneManager->registerNodeForRendering(this, ESNRP_SOLID);
    }
    if (transparentCount) {
      SceneManager->registerNodeForRendering(this, ESNRP_TRANSPARENT);
    }
    ISceneNode::OnRegisterSceneNode();
  }
}
//! renders the node.
void CMeshSceneNode::render()
{
  IVideoDriver* driver = SceneManager->getVideoDriver();
  if (!Mesh || !driver) {
    return;
  }
  bool isTransparentPass =
      SceneManager->getSceneNodeRenderPass() == ESNRP_TRANSPARENT;
  ++PassCount;
  driver->setTransform(ETS_WORLD, AbsoluteTransformation);
  Box = Mesh->getBoundingBox();
  if (Shadow && PassCount == 1) {
    Shadow->updateShadowVolumes();
  }
  // for debug purposes only:
  bool renderMeshes = true;
  SMaterial mat;
  if (DebugDataVisible && PassCount == 1) {
    // overwrite half transparency
    if (DebugDataVisible & EDS_HALF_TRANSPARENCY) {
      for (int g = 0; g < Mesh->getMeshBufferCount(); ++g) {
        mat = Materials[g];
        mat.MaterialType = EMT_TRANSPARENT_ADD_COLOR;
        driver->setMaterial(mat);
        driver->drawMeshBuffer(Mesh->getMeshBuffer(g));
      }
      renderMeshes = false;
    }
  }
  // render original meshes
  if (renderMeshes) {
    int i;
    for (i = 0; i < Mesh->getMeshBufferCount(); ++i) {
      IMeshBuffer* mb = Mesh->getMeshBuffer(i);
      if (mb) {
        const SMaterial& material = ReadOnlyMaterials ? mb->getMaterial() : Materials[i];
        IMaterialRenderer* rnd = driver->getMaterialRenderer(material.MaterialType);
        bool transparent = (rnd && rnd->isTransparent());
        // only render transparent buffer if this is the transparent render pass
        // and solid only in solid pass
        if (transparent == isTransparentPass) {
          driver->setMaterial(material);
          driver->drawMeshBuffer(mb);
        }
      }
    }
  }
  driver->setTransform(ETS_WORLD, AbsoluteTransformation);
  // for debug purposes only:
  if (DebugDataVisible && PassCount == 1) {
    SMaterial m;
    m.Lighting = false;
    m.AntiAliasing = 0;
    driver->setMaterial(m);
    if (DebugDataVisible & EDS_BBOX) {
      driver->draw3DBox(Box, SColor(255, 255, 255, 255));
    }
    if (DebugDataVisible & EDS_BBOX_BUFFERS) {
      for (int g = 0; g < Mesh->getMeshBufferCount(); ++g) {
        driver->draw3DBox(
            Mesh->getMeshBuffer(g)->getBoundingBox(),
            SColor(255, 190, 128, 128));
      }
    }
    if (DebugDataVisible & EDS_NORMALS) {
      // draw normals
      const f32 debugNormalLength = SceneManager->getParameters()->getAttributeAsFloat(DEBUG_NORMAL_LENGTH);
      const SColor debugNormalColor = SceneManager->getParameters()->getAttributeAsColor(DEBUG_NORMAL_COLOR);
      const u32 count = Mesh->getMeshBufferCount();
      int i;
      for (i = 0; i != count; ++i) {
        driver->drawMeshBufferNormals(Mesh->getMeshBuffer(i), debugNormalLength, debugNormalColor);
      }
    }
    // show mesh
    if (DebugDataVisible & EDS_MESH_WIRE_OVERLAY) {
      m.Wireframe = true;
      driver->setMaterial(m);
      for (int g = 0; g < Mesh->getMeshBufferCount(); ++g) {
        driver->drawMeshBuffer(Mesh->getMeshBuffer(g));
      }
    }
  }
}
//! Removes a child from this scene node.
//! Implemented here, to be able to remove the shadow properly, if there is one,
//! or to remove attached childs.
bool CMeshSceneNode::removeChild(ISceneNode* child)
{
  if (child && Shadow == child) {
    Shadow->drop();
    Shadow = 0;
  }
  return ISceneNode::removeChild(child);
}
//! returns the axis aligned bounding box of this node
const F3AABBOX& CMeshSceneNode::getBoundingBox() const
{
  return Mesh ? Mesh->getBoundingBox() : Box;
}
//! returns the material based on the zero based index i. To get the amount
//! of materials used by this scene node, use getMaterialCount().
//! This function is needed for inserting the node into the scene hierarchy on a
//! optimal position for minimizing renderstate changes, but can also be used
//! to directly modify the material of a scene node.
SMaterial& CMeshSceneNode::getMaterial(u32 i)
{
  if (Mesh && ReadOnlyMaterials && i < Mesh->getMeshBufferCount()) {
    ReadOnlyMaterial = Mesh->getMeshBuffer(i)->getMaterial();
    return ReadOnlyMaterial;
  }
  if (i >= Materials_size) {
    return ISceneNode::getMaterial(i);
  }
  return Materials[i];
}
//! returns amount of materials used by this scene node.
u32 CMeshSceneNode::getMaterialCount() const
{
  if (Mesh && ReadOnlyMaterials) {
    return Mesh->getMeshBufferCount();
  }
  return Materials_size;
}
//! Sets a new mesh
void CMeshSceneNode::setMesh(IMesh* mesh)
{
  if (mesh) {
    mesh->grab();
    if (Mesh) {
      Mesh->drop();
    }
    Mesh = mesh;
    copyMaterials();
  }
}
//! Creates shadow volume scene node as child of this node
//! and returns a pointer to it.
IShadowVolumeSceneNode* CMeshSceneNode::addShadowVolumeSceneNode(
    const IMesh* shadowMesh, s32 id, bool zfailmethod, f32 infinity)
{
  if (!SceneManager->getVideoDriver()->queryFeature(EVDF_STENCIL_BUFFER)) {
    return 0;
  }
  if (!shadowMesh) {
    shadowMesh = Mesh;  // if null is given, use the mesh of node
  }
  if (Shadow) {
    Shadow->drop();
  }
  Shadow = new CShadowVolumeSceneNode(shadowMesh, this, SceneManager, id,  zfailmethod, infinity);
  return Shadow;
}
void CMeshSceneNode::copyMaterials()
{
  Materials.clear();
  if (Mesh) {
    SMaterial mat;
    int i;
    for (i = 0; i < Mesh->getMeshBufferCount(); ++i) {
      IMeshBuffer* mb = Mesh->getMeshBuffer(i);
      if (mb) {
        mat = mb->getMaterial();
      }
      Materials.push_back(mat);
    }
  }
}
//! Writes attributes of the scene node.
void CMeshSceneNode::serializeAttributes(IAttributes* out, SAttributeReadWriteOptions* options) const
{
  IMeshSceneNode::serializeAttributes(out, options);
  if (options && (options->Flags & EARWF_USE_RELATIVE_PATHS) && options->Filename) {
    const char* path = SceneManager->getFileSystem()->getRelativeFilename(
        SceneManager->getFileSystem()->getAbsolutePath(SceneManager->getMeshCache()->getMeshName(Mesh).getPath()),
        options->Filename);
    out->addString("Mesh", path);
  }
  else {
    out->addString("Mesh", SceneManager->getMeshCache()->getMeshName(Mesh).getPath());
  }
  out->addBool("ReadOnlyMaterials", ReadOnlyMaterials);
}
//! Reads attributes of the scene node.
void CMeshSceneNode::deserializeAttributes(IAttributes* in, SAttributeReadWriteOptions* options)
{
  char* oldMeshStr = SceneManager->getMeshCache()->getMeshName(Mesh);
  char* newMeshStr = in->getAttributeAsString("Mesh");
  ReadOnlyMaterials = in->getAttributeAsBool("ReadOnlyMaterials");
  if (newMeshStr != "" && oldMeshStr != newMeshStr) {
    IMesh* newMesh = 0;
    IAnimatedMesh* newAnimatedMesh = SceneManager->getMesh(newMeshStr);
    if (newAnimatedMesh) {
      newMesh = newAnimatedMesh->getMesh(0);
    }
    if (newMesh) {
      setMesh(newMesh);
    }
  }
  // optional attribute to assign the hint to the whole mesh
  if (in->existsAttribute("HardwareMappingHint") &&
      in->existsAttribute("HardwareMappingBufferType")) {
    E_HARDWARE_MAPPING mapping = EHM_NEVER;
    E_BUFFER_TYPE bufferType = EBT_NONE;
    char* smapping = in->getAttributeAsString("HardwareMappingHint");
    if (smapping.equals_ignore_case("static")) {
      mapping = EHM_STATIC;
    }
    else if (smapping.equals_ignore_case("dynamic")) {
      mapping = EHM_DYNAMIC;
    }
    else if (smapping.equals_ignore_case("stream")) {
      mapping = EHM_STREAM;
    }
    char* sbufferType = in->getAttributeAsString("HardwareMappingBufferType");
    if (sbufferType.equals_ignore_case("vertex")) {
      bufferType = EBT_VERTEX;
    }
    else if (sbufferType.equals_ignore_case("index")) {
      bufferType = EBT_INDEX;
    }
    else if (sbufferType.equals_ignore_case("vertexindex")) {
      bufferType = EBT_VERTEX_AND_INDEX;
    }
    IMesh* mesh = getMesh();
    if (mesh) {
      mesh->setHardwareMappingHint(mapping, bufferType);
    }
  }
  IMeshSceneNode::deserializeAttributes(in, options);
}
//! Sets if the scene node should not copy the materials of the mesh but use them in a read only style.
/* In this way it is possible to change the materials a mesh causing all mesh scene nodes
referencing this mesh to change too. */
void CMeshSceneNode::setReadOnlyMaterials(bool readonly)
{
  ReadOnlyMaterials = readonly;
}
//! Returns if the scene node should not copy the materials of the mesh but use them in a read only style
bool CMeshSceneNode::isReadOnlyMaterials() const
{
  return ReadOnlyMaterials;
}
//! Creates a clone of this scene node and its children.
ISceneNode* CMeshSceneNode::clone(ISceneNode* newParent, ISceneManager* newManager)
{
  if (!newParent) {
    newParent = Parent;
  }
  if (!newManager) {
    newManager = SceneManager;
  }
  CMeshSceneNode* nb = new CMeshSceneNode(Mesh, newParent,
      newManager, ID, RelativeTranslation, RelativeRotation, RelativeScale);
  nb->cloneMembers(this, newManager);
  nb->ReadOnlyMaterials = ReadOnlyMaterials;
  nb->Materials = Materials;
  nb->Shadow = Shadow;
  if (nb->Shadow) {
    nb->Shadow->grab();
  }
  if (newParent) {
    nb->drop();
  }
  return nb;
}
