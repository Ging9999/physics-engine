#include "physics/SpatialHashGrid.h"

void SpatialHashGrid::clear() {
    m_cells.clear();
}

void SpatialHashGrid::insert(int particleIndex, const Vec3& position) {
    m_cells[getCell(position)].push_back(particleIndex);
}

std::vector<int> SpatialHashGrid::getNeighbours(const Vec3& position) const {
    std::vector<int> result;
    CellKey base = getCell(position);
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            for (int dz = -1; dz <= 1; dz++) {
                CellKey cell = {base.x + dx, base.y + dy, base.z + dz};
                auto it = m_cells.find(cell);
                if (it != m_cells.end()) {
                    result.insert(result.end(), it->second.begin(), it->second.end());
                }
            }
        }
    }
    return result;
}
