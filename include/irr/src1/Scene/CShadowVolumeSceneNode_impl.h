// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h
//#include "CShadowVolumeSceneNode.h"
//#include "ISceneManager.h"
//#include "IVideoDriver.h"
//#include "ICameraSceneNode.h"
//#include "SViewFrustum.h"
//#include "SLight.h"
//#include "os.h"
//! constructor
CShadowVolumeSceneNode::CShadowVolumeSceneNode(const IMesh* shadowMesh, ISceneNode* parent,
    ISceneManager* mgr, s32 id, bool zfailmethod, f32 infinity)
  : IShadowVolumeSceneNode(parent, mgr, id),
    ShadowMesh(0), IndexCount(0), VertexCount(0), ShadowVolumesUsed(0),
    Infinity(infinity), UseZFailMethod(zfailmethod)
{
#ifdef _DEBUG
  setDebugName("CShadowVolumeSceneNode");
#endif
  setShadowMesh(shadowMesh);
  setAutomaticCulling(EAC_OFF);
}
//! destructor
CShadowVolumeSceneNode::~CShadowVolumeSceneNode()
{
  if (ShadowMesh) {
    ShadowMesh->drop();
  }
}
void CShadowVolumeSceneNode::createShadowVolume(const float3 light, bool isDirectional)
{
  SShadowVolume* svp = 0;
  F3AABBOX* bb = 0;
  // builds the shadow volume and adds it to the shadow volume list.
  if (ShadowVolumes_size > ShadowVolumesUsed) {
    // get the next unused buffer
    svp = &ShadowVolumes[ShadowVolumesUsed];
    svp->set_used(0);
    bb = &ShadowBBox[ShadowVolumesUsed];
  }
  else {
    ShadowVolumes.push_back(SShadowVolume());
    svp = &ShadowVolumes.getLast();
    ShadowBBox.push_back(F3AABBOX());
    bb = &ShadowBBox.getLast();
  }
  svp->reallocate(IndexCount * 5);
  ++ShadowVolumesUsed;
  // We use triangle lists
  Edges.set_used(IndexCount * 2);
  u32 numEdges = 0;
  numEdges = createEdgesAndCaps(light, svp, bb);
  // for all edges add the near->far quads
  int i;
  for (i = 0; i < numEdges; ++i) {
    const float3 v1 = Vertices[Edges[2 * i + 0]];
    const float3 v2 = Vertices[Edges[2 * i + 1]];
    const float3 v3(v1 + (v1 - light).normalize()*Infinity);
    const float3 v4(v2 + (v2 - light).normalize()*Infinity);
    // Add a quad (two triangles) to the vertex list
#ifdef _DEBUG
    if (svp->size() >= svp->allocated_size() - 5) {
      os::Printer::log("Allocation too small.", ELL_DEBUG);
    }
#endif
    svp->push_back(v1);
    svp->push_back(v2);
    svp->push_back(v3);
    svp->push_back(v2);
    svp->push_back(v4);
    svp->push_back(v3);
  }
}
#define IRR_USE_ADJACENCY
#define IRR_USE_REVERSE_EXTRUDED
u32 CShadowVolumeSceneNode::createEdgesAndCaps(const float3 light,
    SShadowVolume* svp, F3AABBOX* bb)
{
  u32 numEdges = 0;
  const u32 faceCount = IndexCount / 3;
  if (faceCount >= 1) {
    bb->reset(Vertices[Indices[0]]);
  }
  else {
    bb->reset(0, 0, 0);
  }
  // Check every face if it is front or back facing the light.
  int i;
  for (i = 0; i < faceCount; ++i) {
    const float3 v0 = Vertices[Indices[3 * i + 0]];
    const float3 v1 = Vertices[Indices[3 * i + 1]];
    const float3 v2 = Vertices[Indices[3 * i + 2]];
#ifdef IRR_USE_REVERSE_EXTRUDED
    FaceData[i] = triangle3df(v0, v1, v2).isFrontFacing(light);
#else
    FaceData[i] = triangle3df(v2, v1, v0).isFrontFacing(light);
#endif
    if (UseZFailMethod && FaceData[i]) {
#ifdef _DEBUG
      if (svp->size() >= svp->allocated_size() - 5) {
        os::Printer::log("Allocation too small.", ELL_DEBUG);
      }
#endif
      // add front cap from light-facing faces
      svp->push_back(v2);
      svp->push_back(v1);
      svp->push_back(v0);
      // add back cap
      const float3 i0 = v0 + (v0 - light).normalize() * Infinity;
      const float3 i1 = v1 + (v1 - light).normalize() * Infinity;
      const float3 i2 = v2 + (v2 - light).normalize() * Infinity;
      svp->push_back(i0);
      svp->push_back(i1);
      svp->push_back(i2);
      bb->addInternalPoint(i0);
      bb->addInternalPoint(i1);
      bb->addInternalPoint(i2);
    }
  }
  // Create edges
  for (i = 0; i < faceCount; ++i) {
    // check all front facing faces
    if (FaceData[i] == true) {
      const u16 wFace0 = Indices[3 * i + 0];
      const u16 wFace1 = Indices[3 * i + 1];
      const u16 wFace2 = Indices[3 * i + 2];
      const u16 adj0 = Adjacency[3 * i + 0];
      const u16 adj1 = Adjacency[3 * i + 1];
      const u16 adj2 = Adjacency[3 * i + 2];
      // add edges if face is adjacent to back-facing face
      // or if no adjacent face was found
#ifdef IRR_USE_ADJACENCY
      if (adj0 == i || FaceData[adj0] == false)
#endif
      {
        // add edge v0-v1
        Edges[2 * numEdges + 0] = wFace0;
        Edges[2 * numEdges + 1] = wFace1;
        ++numEdges;
      }
#ifdef IRR_USE_ADJACENCY
      if (adj1 == i || FaceData[adj1] == false)
#endif
      {
        // add edge v1-v2
        Edges[2 * numEdges + 0] = wFace1;
        Edges[2 * numEdges + 1] = wFace2;
        ++numEdges;
      }
#ifdef IRR_USE_ADJACENCY
      if (adj2 == i || FaceData[adj2] == false)
#endif
      {
        // add edge v2-v0
        Edges[2 * numEdges + 0] = wFace2;
        Edges[2 * numEdges + 1] = wFace0;
        ++numEdges;
      }
    }
  }
  return numEdges;
}
void CShadowVolumeSceneNode::setShadowMesh(const IMesh* mesh)
{
  if (ShadowMesh == mesh) {
    return;
  }
  if (ShadowMesh) {
    ShadowMesh->drop();
  }
  ShadowMesh = mesh;
  if (ShadowMesh) {
    ShadowMesh->grab();
    Box = ShadowMesh->getBoundingBox();
  }
}
void CShadowVolumeSceneNode::updateShadowVolumes()
{
  const u32 oldIndexCount = IndexCount;
  const u32 oldVertexCount = VertexCount;
  const IMesh* const mesh = ShadowMesh;
  if (!mesh) {
    return;
  }
  // create as much shadow volumes as there are lights but
  // do not ignore the max light settings.
  const u32 lightCount = SceneManager->getVideoDriver()->getDynamicLightCount();
  if (!lightCount) {
    return;
  }
  // calculate total amount of vertices and indices
  VertexCount = 0;
  IndexCount = 0;
  ShadowVolumesUsed = 0;
  u32 i;
  u32 totalVertices = 0;
  u32 totalIndices = 0;
  const u32 bufcnt = mesh->getMeshBufferCount();
  for (i = 0; i < bufcnt; ++i) {
    const IMeshBuffer* buf = mesh->getMeshBuffer(i);
    totalIndices += buf->getIndexCount();
    totalVertices += buf->getVertexCount();
  }
  // allocate memory if necessary
  Vertices.set_used(totalVertices);
  Indices.set_used(totalIndices);
  FaceData.set_used(totalIndices / 3);
  // copy mesh
  for (i = 0; i < bufcnt; ++i) {
    const IMeshBuffer* buf = mesh->getMeshBuffer(i);
    const u16* idxp = buf->getIndices();
    const u16* idxpend = idxp + buf->getIndexCount();
    for (; idxp != idxpend; ++idxp) {
      Indices[IndexCount++] = *idxp + VertexCount;
    }
    const u32 vtxcnt = buf->getVertexCount();
    for (int j = 0; j < vtxcnt; ++j) {
      Vertices[VertexCount++] = buf->getPosition(j);
    }
  }
  // recalculate adjacency if necessary
  if (oldVertexCount != VertexCount || oldIndexCount != IndexCount) {
    calculateAdjacency();
  }
  matrix4 mat = Parent->getAbsoluteTransformation();
  mat.makeInverse();
  const float3 parentpos = Parent->getAbsolutePosition();
  // TODO: Only correct for point lights.
  for (i = 0; i < lightCount; ++i) {
    const SLight& dl = SceneManager->getVideoDriver()->getDynamicLight(i);
    float3 lpos = dl.Position;
    if (dl.CastShadows &&
        fabs((lpos - parentpos).getLengthSQ()) <= (dl.Radius * dl.Radius * 4.0f)) {
      mat.transformVect(lpos);
      createShadowVolume(lpos);
    }
  }
}
//! pre render method
void CShadowVolumeSceneNode::OnRegisterSceneNode()
{
  if (IsVisible) {
    SceneManager->registerNodeForRendering(this, ESNRP_SHADOW);
    ISceneNode::OnRegisterSceneNode();
  }
}
//! renders the node.
void CShadowVolumeSceneNode::render()
{
  IVideoDriver* driver = SceneManager->getVideoDriver();
  if (!ShadowVolumesUsed || !driver) {
    return;
  }
  driver->setTransform(ETS_WORLD, Parent->getAbsoluteTransformation());
  int i;
  for (i = 0; i < ShadowVolumesUsed; ++i) {
    bool drawShadow = true;
    if (UseZFailMethod && SceneManager->getActiveCamera()) {
      // Disable shadows drawing, when back cap is behind of ZFar plane.
      SViewFrustum frust = *SceneManager->getActiveCamera()->getViewFrustum();
      matrix4 invTrans(Parent->getAbsoluteTransformation(), matrix4::EM4CONST_INVERSE);
      frust.transform(invTrans);
      float3 edges[8];
      ShadowBBox[i].getEdges(edges);
      float3 largestEdge = edges[0];
      f32 maxDistance = float3(SceneManager->getActiveCamera()->getPosition() - edges[0]).getLength();
      f32 curDistance = 0.f;
      for (int j = 1; j < 8; ++j) {
        curDistance = float3(SceneManager->getActiveCamera()->getPosition() - edges[j]).getLength();
        if (curDistance > maxDistance) {
          maxDistance = curDistance;
          largestEdge = edges[j];
        }
      }
      if (!(frust.planes[SViewFrustum::VF_FAR_PLANE].classifyPointRelation(largestEdge) != ISREL3D_FRONT)) {
        drawShadow = false;
      }
    }
    if (drawShadow) {
      driver->drawStencilShadowVolume(ShadowVolumes[i], UseZFailMethod, DebugDataVisible);
    }
    else {
      float3 > triangles;
      driver->drawStencilShadowVolume(triangles, UseZFailMethod, DebugDataVisible);
    }
  }
}
//! returns the axis aligned bounding box of this node
const F3AABBOX& CShadowVolumeSceneNode::getBoundingBox() const
{
  return Box;
}
//! Generates adjacency information based on mesh indices.
void CShadowVolumeSceneNode::calculateAdjacency()
{
  Adjacency.set_used(IndexCount);
  // go through all faces and fetch their three neighbours
  for (int f = 0; f < IndexCount; f += 3) {
    for (int edge = 0; edge < 3; ++edge) {
      const float3 v1 = Vertices[Indices[f + edge]];
      const float3 v2 = Vertices[Indices[f + ((edge + 1) % 3)]];
      // now we search an_O_ther _F_ace with these two
      // vertices, which is not the current face.
      u32 of;
      for (of = 0; of < IndexCount; of += 3) {
        // only other faces
        if (of != f) {
          bool cnt1 = false;
          bool cnt2 = false;
          for (s32 e = 0; e < 3; ++e) {
            if (v1.equals(Vertices[Indices[of + e]])) {
              cnt1 = true;
            }
            if (v2.equals(Vertices[Indices[of + e]])) {
              cnt2 = true;
            }
          }
          // one match for each vertex, i.e. edge is the same
          if (cnt1 && cnt2) {
            break;
          }
        }
      }
      // no adjacent edges -> store face number, else store adjacent face
      if (of >= IndexCount) {
        Adjacency[f + edge] = f / 3;
      }
      else {
        Adjacency[f + edge] = of / 3;
      }
    }
  }
}
