#include "Pxl/Pxl/interface/pxl/core/BasicMatrix.hh"
#include <cmath>
namespace pxl
{

/**
 * The rotation Matrix is a 3x3 Basic Matrix constructed by an rotation
 * axis and an rotation angle
 */
class PXL_DLL_EXPORT RotationMatrix : public BasicMatrix
{
  public:
    RotationMatrix() : BasicMatrix(3, 3){};
    RotationMatrix(const RotationMatrix &orig) : BasicMatrix(orig){};

    explicit RotationMatrix(const RotationMatrix *orig) : BasicMatrix(*orig){};

    RotationMatrix(const Basic3Vector &rotationAxis, double rotationAngle) : BasicMatrix(3, 3)
    {
        double s = std::sin(rotationAngle);
        double c = std::cos(rotationAngle);
        _data[0] = rotationAxis.getX() * rotationAxis.getX() + (1 - rotationAxis.getX() * rotationAxis.getX()) * c;
        _data[1] = rotationAxis.getX() * rotationAxis.getY() * (1 - c) - rotationAxis.getZ() * s;
        _data[2] = rotationAxis.getX() * rotationAxis.getZ() * (1 - c) + rotationAxis.getY() * s;
        _data[3] = rotationAxis.getX() * rotationAxis.getY() * (1 - c) + rotationAxis.getZ() * s;
        _data[4] = rotationAxis.getY() * rotationAxis.getY() + (1 - rotationAxis.getY() * rotationAxis.getY()) * c;
        _data[5] = rotationAxis.getY() * rotationAxis.getZ() * (1 - c) - rotationAxis.getX() * s;
        _data[6] = rotationAxis.getX() * rotationAxis.getZ() * (1 - c) - rotationAxis.getY() * s;
        _data[7] = rotationAxis.getY() * rotationAxis.getZ() * (1 - c) + rotationAxis.getX() * s;
        _data[8] = rotationAxis.getZ() * rotationAxis.getZ() + (1 - rotationAxis.getZ() * rotationAxis.getZ()) * c;
    }
};

} // namespace pxl
