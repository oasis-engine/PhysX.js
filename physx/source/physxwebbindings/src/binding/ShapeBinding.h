//
// Created by Oasis on 2023/3/21.
//

#pragma once

#include <emscripten.h>
#include <emscripten/bind.h>

#include "../SimulationFilterShader.h"
#include "PxPhysicsAPI.h"
#include "QueryBinding.h"

using namespace physx;
using namespace emscripten;

EMSCRIPTEN_BINDINGS(physx_shape) {
    function("PxCreatePlane", &PxCreatePlane, allow_raw_pointers());

    // enum_<PxCombineMode::Enum>("PxCombineMode")
    //         .value("eAVERAGE", PxCombineMode::Enum::eAVERAGE)
    //         .value("eMIN", PxCombineMode::Enum::eMIN)
    //         .value("eMAX", PxCombineMode::Enum::eMAX)
    //         .value("eMULTIPLY", PxCombineMode::Enum::eMULTIPLY);
    class_<PxMaterial>("PxMaterial") // ✅
            .function("release", &PxMaterial::release) // ✅
            .function("setDynamicFriction", &PxMaterial::setDynamicFriction) // ✅
            .function("setStaticFriction", &PxMaterial::setStaticFriction) // ✅
            .function("setRestitution", &PxMaterial::setRestitution) // ✅
            .function("setFrictionCombineMode", optional_override([](PxMaterial &material, int mode) {
                          return material.setFrictionCombineMode(PxCombineMode::Enum(mode));
                      })) // ✅
            .function("setRestitutionCombineMode", optional_override([](PxMaterial &material, int mode) {
                          return material.setRestitutionCombineMode(PxCombineMode::Enum(mode));
                      })); // ✅

    register_vector<PxMaterial *>("VectorPxMaterial"); // ✅
    // setMaterials has 'PxMaterial**' as an input, which is not representable with embind
    // This is overrided to use std::vector<PxMaterial*>
    class_<PxShape>("PxShape") // ✅
            .function("release", optional_override([](PxShape &shape) {
                          if (shape.userData) {
                              free(shape.userData);
                              shape.userData = nullptr;
                          }
                          shape.release();
                      })) // ✅
            .function("getGlobalPose", optional_override([](PxShape &shape) {
                          PxActor* actor = shape.getActor();
                          if (actor){
                            return shape.getActor()->getGlobalPose() * shape.getLocalPose();
                          } else {
                            return shape.getLocalPose();
                          }
                      })) // ✅
            .function("setContactOffset", &PxShape::setContactOffset) // ✅
            // .function("getContactOffset", &PxShape::getContactOffset) 
            // .function("getFlags", &PxShape::getFlags) 
            // .function("setFlag", &PxShape::setFlag)
            .function("setFlags", &PxShape::setFlags) // ✅
            .function("setLocalPose", &PxShape::setLocalPose) // ✅
            .function("setGeometry", &PxShape::setGeometry) // ✅
            // .function("getBoxGeometry", &PxShape::getBoxGeometry, allow_raw_pointers())
            // .function("getSphereGeometry", &PxShape::getSphereGeometry, allow_raw_pointers())
            // .function("getPlaneGeometry", &PxShape::getPlaneGeometry, allow_raw_pointers())
            // .function("getCapsuleGeometry", &PxShape::getCapsuleGeometry, allow_raw_pointers())
            // .function("setSimulationFilterData", &PxShape::setSimulationFilterData, allow_raw_pointers())
            .function("setMaterial", optional_override([](PxShape &shape, PxMaterial& material) {
                          std::vector<PxMaterial*> materialsVector;
                          materialsVector.push_back(&material);
                          return shape.setMaterials(materialsVector.data(), materialsVector.size());
                      }), allow_raw_pointers()) // ✅
            // .function("setMaterials", optional_override([](PxShape &shape, val materials) {
            //               int length = materials["length"].as<int>();
            //               std::vector<PxMaterial*> materialsVector;
            //               for(int i = 0; i < length; i++) {
            //                 PxMaterial* ptr = reinterpret_cast<PxMaterial*>((materials[i]["$$"]["ptr"]).as<int>());
            //                 materialsVector.push_back(ptr);
            //               }
            //               return shape.setMaterials(materialsVector.data(), materialsVector.size());
            //           }))
            .function("setUUID", optional_override([](PxShape &shape, uint32_t uuid) {
                          auto ptr = malloc(sizeof(uint32_t));
                          memcpy(ptr, &uuid, sizeof(uint32_t));
                          shape.userData = ptr;
                      })) // ✅
            .function("getUUID", optional_override([](PxShape &shape) {
                          return getUUID(&shape);
                      }));// ✅

    class_<PxShapeFlags>("PxShapeFlags").constructor<int>();  // ✅
    // .function("isSet", &PxShapeFlags::isSet);
    // enum_<PxShapeFlag::Enum>("PxShapeFlag")
    //         .value("eSIMULATION_SHAPE", PxShapeFlag::Enum::eSIMULATION_SHAPE)
    //         .value("eSCENE_QUERY_SHAPE", PxShapeFlag::Enum::eSCENE_QUERY_SHAPE)
    //         .value("eTRIGGER_SHAPE", PxShapeFlag::Enum::eTRIGGER_SHAPE)
    //         .value("eVISUALIZATION", PxShapeFlag::Enum::eVISUALIZATION);

    /** PhysXColliderShape ✅ */
    class_<PxGeometry>("PxGeometry")
            .function("pointDistance", optional_override([](PxGeometry &geometry,
            PxTransform &pose, PxVec3 &point) {
                PxVec3 closestPoint = PxVec3(0.0f);
                PxReal distance = PxGeometryQuery::pointDistance(point, geometry, pose, &closestPoint);
                          val res = val::object();
                          res.set("distance", distance);
                          res.set("closestPoint", closestPoint);
                          return res;
                      })); // ✅
    /** PhysXBoxColliderShape ✅ */
    class_<PxBoxGeometry, base<PxGeometry>>("PxBoxGeometry")
            .constructor<>()
            .constructor<float, float, float>()
            // .function("isValid", &PxBoxGeometry::isValid)
            .property("halfExtents", &PxBoxGeometry::halfExtents);  // ✅
    /** PhysXSphereColliderShape ✅ */
    class_<PxSphereGeometry, base<PxGeometry>>("PxSphereGeometry")
            .constructor<>()
            .constructor<float>()
            .property("radius", &PxSphereGeometry::radius);  // ✅
            // .function("isValid", &PxSphereGeometry::isValid);
    /** PhysXCapsuleColliderShape ✅ */
    class_<PxCapsuleGeometry, base<PxGeometry>>("PxCapsuleGeometry")
            .constructor<float, float>()
            .property("radius", &PxCapsuleGeometry::radius)          // ✅
            .property("halfHeight", &PxCapsuleGeometry::halfHeight);  // ✅
            // .function("isValid", &PxCapsuleGeometry::isValid);
    /** PhysXCapsuleColliderShape ✅ */
    class_<PxPlaneGeometry, base<PxGeometry>>("PxPlaneGeometry").constructor<>();

    class_<PxPlane>("PxPlane").constructor<float, float, float, float>();
}

namespace emscripten {
namespace internal {
template <>
void raw_destructor<PxMaterial>(PxMaterial *) { /* do nothing */
}
template <>
void raw_destructor<PxShape>(PxShape *) { /* do nothing */
}

}  // namespace internal
}  // namespace emscripten
