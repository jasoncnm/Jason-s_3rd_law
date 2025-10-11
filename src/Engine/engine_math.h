#if !defined(RAYLIB_WRAPPER_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#include "engine_lib.h"

// Utils math
float EngineClamp(float value, float min, float max);                             // Clamp float value
float EngineLerp(float start, float end, float amount);                           // Calculate linear interpolation between two floats
float EngineNormalize(float value, float start, float end);                       // Normalize input value within input range
float EngineRemap(float value, float inputStart, float inputEnd, float outputStart, float outputEnd); // Remap input value within input range to output range
float EngineWrap(float value, float min, float max);                              // Wrap input value from min to max
int   EngineFloatEquals(float x, float y);                                          // Check whether two given floats are almost equal
Vec4  EngineFade(Vec4 color, float alpha);                                 // Get color with alpha applied, alpha goes from 0.0f to 1.0f

// Vec2 math
Vec2 Vec2Zero(void);                                                  // Vec with components value 0.0f
Vec2 Vec2One(void);                                                   // Vec with components value 1.0f
Vec2 Vec2Add(Vec2 v1, Vec2 v2);                                 // Add two vectors (v1 + v2)
Vec2 Vec2AddValue(Vec2 v, float add);                              // Add vector and float value
Vec2 Vec2Subtract(Vec2 v1, Vec2 v2);                            // Subtract two vectors (v1 - v2)
Vec2 Vec2SubtractValue(Vec2 v, float sub);                         // Subtract vector by float value
float Vec2Length(Vec2 v);                                             // Calculate vector length
float Vec2LengthSqr(Vec2 v);                                          // Calculate vector square length
float Vec2DotProduct(Vec2 v1, Vec2 v2);                            // Calculate two vectors dot product
float Vec2Distance(Vec2 v1, Vec2 v2);                              // Calculate distance between two vectors
float Vec2DistanceSqr(Vec2 v1, Vec2 v2);                           // Calculate square distance between two vectors
float Vec2Angle(Vec2 v1, Vec2 v2);                                 // Calculate angle from two vectors
Vec2 Vec2Scale(Vec2 v, float scale);                               // Scale vector (multiply by value)
Vec2 Vec2Multiply(Vec2 v1, Vec2 v2);                            // Multiply vector by vector
Vec2 Vec2Negate(Vec2 v);                                           // Negate vector
Vec2 Vec2Divide(Vec2 v1, Vec2 v2);                              // Divide vector by vector
Vec2 Vec2Normalize(Vec2 v);                                        // Normalize provided vector
Vec2 Vec2Transform(Vec2 v, Mat4 mat);                            // Transforms a Vec2 by a given Matrix
Vec2 Vec2Lerp(Vec2 v1, Vec2 v2, float amount);                  // Calculate linear interpolation between two vectors
Vec2 Vec2Reflect(Vec2 v, Vec2 normal);                          // Calculate reflected vector to normal
Vec2 Vec2Rotate(Vec2 v, float angle);                              // Rotate vector by angle
Vec2 Vec2MoveTowards(Vec2 v, Vec2 target, float maxDistance);   // Move Vec towards target
Vec2 Vec2Invert(Vec2 v);                                           // Invert the given vector
Vec2 Vec2Clamp(Vec2 v, Vec2 min, Vec2 max);                  // Clamp the components of the vector between min and max values specified by the given vectors
Vec2 Vec2ClampValue(Vec2 v, float min, float max);                 // Clamp the magnitude of the vector between two min and max values
int Vec2Equals(Vec2 p, Vec2 q);                                    // Check whether two given vectors are almost equal

// Vec3 math
Vec3 Vec3Zero(void);                                                  // Vec with components value 0.0f
Vec3 Vec3One(void);                                                   // Vec with components value 1.0f
Vec3 Vec3Add(Vec3 v1, Vec3 v2);                                 // Add two vectors
Vec3 Vec3AddValue(Vec3 v, float add);                              // Add vector and float value
Vec3 Vec3Subtract(Vec3 v1, Vec3 v2);                            // Subtract two vectors
Vec3 Vec3SubtractValue(Vec3 v, float sub);                         // Subtract vector by float value
Vec3 Vec3Scale(Vec3 v, float scalar);                              // Multiply vector by scalar
Vec3 Vec3Multiply(Vec3 v1, Vec3 v2);                            // Multiply vector by vector
Vec3 Vec3CrossProduct(Vec3 v1, Vec3 v2);                        // Calculate two vectors cross product
Vec3 Vec3Perpendicular(Vec3 v);                                    // Calculate one vector perpendicular vector
float Vec3Length(const Vec3 v);                                       // Calculate vector length
float Vec3LengthSqr(const Vec3 v);                                    // Calculate vector square length
float Vec3DotProduct(Vec3 v1, Vec3 v2);                            // Calculate two vectors dot product
float Vec3Distance(Vec3 v1, Vec3 v2);                              // Calculate distance between two vectors
float Vec3DistanceSqr(Vec3 v1, Vec3 v2);                           // Calculate square distance between two vectors
float Vec3Angle(Vec3 v1, Vec3 v2);                                 // Calculate angle between two vectors
Vec3 Vec3Negate(Vec3 v);                                           // Negate provided vector (invert direction)
Vec3 Vec3Divide(Vec3 v1, Vec3 v2);                              // Divide vector by vector
Vec3 Vec3Normalize(Vec3 v);                                        // Normalize provided vector
void Vec3OrthoNormalize(Vec3 *v1, Vec3 *v2);                       // Orthonormalize provided vectors Makes vectors normalized and orthogonal to each other Gram-Schmidt function implementation
Vec3 Vec3Transform(Vec3 v, Mat4 mat);                            // Transforms a Vec3 by a given Matrix
Vec3 Vec3RotateByQuat(Vec3 v, Quat q);                 // Transform a vector by quaternion rotation
Vec3 Vec3RotateByAxisAngle(Vec3 v, Vec3 axis, float angle);     // Rotates a vector around an axis
Vec3 Vec3Lerp(Vec3 v1, Vec3 v2, float amount);                  // Calculate linear interpolation between two vectors
Vec3 Vec3Reflect(Vec3 v, Vec3 normal);                          // Calculate reflected vector to normal
Vec3 Vec3Min(Vec3 v1, Vec3 v2);                                 // Get min value for each pair of components
Vec3 Vec3Max(Vec3 v1, Vec3 v2);                                 // Get max value for each pair of components
Vec3 Vec3Barycenter(Vec3 p, Vec3 a, Vec3 b, Vec3 c);      // Compute barycenter coordinates (u, v, w) for point p with respect to triangle (a, b, c) NOTE: Assumes P is on the plane of the triangle
Vec3 Vec3Unproject(Vec3 source, Mat4 projection, Mat4 view);   // Projects a Vec3 from screen space into object space NOTE: We are avoiding calling other raymath functions despite available
Array<float, 3> Vec3ToFloatV(Vec3 v);                                          // Get Vec3 as float array
Vec3 Vec3Invert(Vec3 v);                                           // Invert the given vector
Vec3 Vec3Clamp(Vec3 v, Vec3 min, Vec3 max);                  // Clamp the components of the vector between min and max values specified by the given vectors
Vec3 Vec3ClampValue(Vec3 v, float min, float max);                 // Clamp the magnitude of the vector between two values
int Vec3Equals(Vec3 p, Vec3 q);                                    // Check whether two given vectors are almost equal
Vec3 Vec3Refract(Vec3 v, Vec3 n, float r);                      // Compute the direction of a refracted ray where v specifies the normalized direction of the incoming ray, n specifies the normalized normal vector of the interface of two optical media, and r specifies the ratio of the refractive index of the medium from where the ray comes to the refractive index of the medium on the other side of the surface

// Matrix math
float Mat4Determinant(Mat4 mat);                                        // Compute matrix determinant
float Mat4Trace(Mat4 mat);                                              // Get the trace of the matrix (sum of the values along the diagonal)
Mat4 Mat4Transpose(Mat4 mat);                                         // Transposes provided matrix
Mat4 Mat4Invert(Mat4 mat);                                            // Invert provided matrix
Mat4 Mat4Identity(void);                                                // Get identity matrix
Mat4 Mat4Add(Mat4 left, Mat4 right);                                // Add two matrices
Mat4 Mat4Subtract(Mat4 left, Mat4 right);                           // Subtract two matrices (left - right)
Mat4 Mat4Multiply(Mat4 left, Mat4 right);                           // Get two matrix multiplication NOTE: When multiplying matrices... the order matters!
Mat4 Mat4Translate(float x, float y, float z);                          // Get translation matrix
Mat4 Mat4Rotate(Vec3 axis, float angle);                             // Create rotation matrix from axis and angle NOTE: Angle should be provided in radians
Mat4 Mat4RotateX(float angle);                                          // Get x-rotation matrix NOTE: Angle must be provided in radians
Mat4 Mat4RotateY(float angle);                                          // Get y-rotation matrix NOTE: Angle must be provided in radians
Mat4 Mat4RotateZ(float angle);                                          // Get z-rotation matrix NOTE: Angle must be provided in radians
Mat4 Mat4RotateXYZ(Vec3 angle);                                      // Get xyz-rotation matrix NOTE: Angle must be provided in radians
Mat4 Mat4RotateZYX(Vec3 angle);                                      // Get zyx-rotation matrix NOTE: Angle must be provided in radians
Mat4 Mat4Scale(float x, float y, float z);                              // Get scaling matrix
Mat4 Mat4Frustum(double left, double right, double bottom, double top, double near, double far); // Get perspective projection matrix
Mat4 Mat4Perspective(double fovy, double aspect, double near, double far); // Get perspective projection matrix NOTE: Fovy angle must be provided in radians
Mat4 Mat4Ortho(double left, double right, double bottom, double top, double near, double far); // Get orthographic projection matrix
Mat4 Mat4LookAt(Vec3 eye, Vec3 target, Vec3 up);               // Get camera look-at matrix (view matrix)
Array<float, 16> Mat4ToFloatV(Mat4 mat);                                         // Get float array of matrix data

// Quat math
Quat QuatAdd(Quat q1, Quat q2);                     // Add two quaternions
Quat QuatAddValue(Quat q, float add);                     // Add quaternion and float value
Quat QuatSubtract(Quat q1, Quat q2);                // Subtract two quaternions
Quat QuatSubtractValue(Quat q, float sub);                // Subtract quaternion and float value
Quat QuatIdentity(void);                                        // Get identity quaternion
float QuatLength(Quat q);                                       // Computes the length of a quaternion
Quat QuatNormalize(Quat q);                               // Normalize provided quaternion
Quat QuatInvert(Quat q);                                  // Invert provided quaternion
Quat QuatMultiply(Quat q1, Quat q2);                // Calculate two quaternion multiplication
Quat QuatScale(Quat q, float mul);                        // Scale quaternion by float value
Quat QuatDivide(Quat q1, Quat q2);                  // Divide two quaternions
Quat QuatLerp(Quat q1, Quat q2, float amount);      // Calculate linear interpolation between two quaternions
Quat QuatNlerp(Quat q1, Quat q2, float amount);     // Calculate slerp-optimized interpolation between two quaternions
Quat QuatSlerp(Quat q1, Quat q2, float amount);     // Calculates spherical linear interpolation between two quaternions
Quat QuatFromVec3ToVec3(Vec3 from, Vec3 to);        // Calculate quaternion based on the rotation from one vector to another
Quat QuatFromMat4(Mat4 mat);                                // Get a quaternion for a given rotation matrix
Mat4 QuatToMat4(Quat q);                                    // Get a matrix for a given quaternion
Quat QuatFromAxisAngle(Vec3 axis, float angle);              // Get rotation quaternion for an angle and axis NOTE: Angle must be provided in radians
void QuatToAxisAngle(Quat q, Vec3 *outAxis, float *outAngle); // Get the rotation angle and axis for a given quaternion
Quat QuatFromEuler(float pitch, float yaw, float roll);         // Get the quaternion equivalent to Euler angles NOTE: Rotation order is ZYX
Vec3 QuatToEuler(Quat q);                                    // Get the Euler angles equivalent to quaternion (roll, pitch, yaw) NOTE: Angles are returned in a Vec3 struct in radians
Quat QuatTransform(Quat q, Mat4 mat);                   // Transform a quaternion given a transformation matrix
int QuatEquals(Quat p, Quat q);                           // Check whether two given quaternions are almost equal

#define RAYLIB_WRAPPER_H
#endif
