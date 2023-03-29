#include <VRDummyCamera.h>

static kvs::Camera s_dummy_camera;
static bool is_initialized = false;

kvs::Camera* dummyCameraForVR () {
    if (!is_initialized) {
        // set dummy camera parameters.
        s_dummy_camera.setPosition(kvs::Vec3(0.0, 0.0, 12.0));
        s_dummy_camera.setUpVector(kvs::Vec3(0.0, 1.0, 0.0));
        s_dummy_camera.setLookAt(kvs::Vec3(0.0, 0.0, 0.0));

        s_dummy_camera.setXform(kvs::Xform(kvs::Mat4(
                                    1.0, 0.0, 0.0, 0.0,
                                    0.0, 1.0, 0.0, 0.0,
                                    0.0, 0.0, 1.0, 12.0,
                                    0.0, 0.0, 0.0, 1.0
                                    )));

        s_dummy_camera.setProjectionType(kvs::Camera::ProjectionType::Perspective);
        s_dummy_camera.setFieldOfView(45.0);
        s_dummy_camera.setFront(1.0);
        s_dummy_camera.setBack(2000.0);
        s_dummy_camera.setLeft(-5.0);
        s_dummy_camera.setRight(5.0);
        s_dummy_camera.setTop(5.0);
        s_dummy_camera.setBottom(-5.0);
        s_dummy_camera.setWindowSize(620, 620);
        //m_dummy_camera.setWindowSize(1328,1584);

        is_initialized = true;
    }

    return &s_dummy_camera;
}
