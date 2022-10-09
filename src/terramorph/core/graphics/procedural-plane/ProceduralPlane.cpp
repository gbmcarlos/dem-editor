#include "terramorph/core/graphics/procedural-plane/ProceduralPlane.h"

namespace terramorph::Core {

    std::vector<PlaneQuad> QuadTreePatch::compute(float planeWidth, float planeHeight, float targetResolution, float resolutionSlope, const glm::vec3& cameraPosition, const gaunlet::Scene::Frustum& cameraFrustum) {

        Context context(planeWidth, planeHeight, targetResolution, resolutionSlope, cameraPosition, cameraFrustum);

        auto rootNode = gaunlet::Core::CreateRef<QuadTreePatch>(nullptr, PatchPosition::Root, context, glm::vec3(0, 0, 0), planeWidth, planeHeight);
        rootNode->process();

        for (auto& quad : context.m_quads) {
            quad.m_patch->processEdges(quad);
        }

        return context.m_quads;

    }

    void QuadTreePatch::computeDimensions() {

        m_leftEdge = m_origin.x - (m_width / 2);
        m_rightEdge = m_origin.x + (m_width / 2);
        m_bottomEdge = m_origin.z + (m_height / 2); // Positive, because this is the Z axis
        m_topEdge = m_origin.z - (m_height / 2); // Negative, because this is the Z axis

    }

    void QuadTreePatch::process() {

        // If this patch requires subdivision, there's no need to create its own content
        if (requiresSubdivision()) {
            subdivide();
        } else {
            createContent();
        }

    }

    bool QuadTreePatch::requiresSubdivision() {

        glm::vec3 projectedCameraPosition(m_context.m_cameraPosition.x, 0, m_context.m_cameraPosition.z + 1.0);

        // Calculate the position of the 4 corners of the quad
        glm::vec3 leftBottomCorner = {m_leftEdge, 0, m_bottomEdge};
        glm::vec3 rightBottomCorner = {m_rightEdge, 0, m_bottomEdge};
        glm::vec3 rightTopCorner = {m_rightEdge, 0, m_topEdge};
        glm::vec3 leftTopCorner = {m_leftEdge, 0, m_topEdge};

        // The distance between the camera and each corner of the quad
        float distanceLeftBottom = glm::length(leftBottomCorner - projectedCameraPosition);
        float distanceRightBottom = glm::length(rightBottomCorner - projectedCameraPosition);
        float distanceRightTop = glm::length(rightTopCorner - projectedCameraPosition);
        float distanceLeftTop = glm::length(leftTopCorner - projectedCameraPosition);

        // The distance to the closest corner
        float distance = std::min(std::min(distanceLeftBottom, distanceRightBottom), std::min(distanceRightTop, distanceLeftTop));

        // Calculate the required resolution based on the distance to the camera
        float relativeResolution = getRelativeResolution(distance);

        return m_width > relativeResolution || m_height > relativeResolution;

    }

    float QuadTreePatch::getRelativeResolution(float distance) const {

        float relativeResolution = (distance * m_context.m_resolutionSlope) + 1;
        relativeResolution = std::max(relativeResolution, m_context.m_targetResolution);

        return relativeResolution;

    }

    // Add 4 children to the node, each half the size
    void QuadTreePatch::subdivide() {

        auto leftBottomPatch = gaunlet::Core::CreateRef<QuadTreePatch>(this, PatchPosition::LeftBottom, m_context, glm::vec3(m_origin.x - (m_width/4), 0, m_origin.z + (m_height/4)), m_width/2, m_height/2);
        m_children.emplace_back(leftBottomPatch);
        leftBottomPatch->process();

        auto rightBottomPatch = gaunlet::Core::CreateRef<QuadTreePatch>(this, PatchPosition::RightBottom, m_context, glm::vec3(m_origin.x + (m_width/4), 0, m_origin.z + (m_height/4)), m_width/2, m_height/2);
        m_children.emplace_back(rightBottomPatch);
        rightBottomPatch->process();

        auto rightTopPatch = gaunlet::Core::CreateRef<QuadTreePatch>(this, PatchPosition::RightTop, m_context, glm::vec3(m_origin.x + (m_width/4), 0, m_origin.z - (m_height/4)), m_width/2, m_height/2);
        m_children.emplace_back(rightTopPatch);
        rightTopPatch->process();

        auto leftTopPatch = gaunlet::Core::CreateRef<QuadTreePatch>(this, PatchPosition::LeftTop, m_context, glm::vec3(m_origin.x - (m_width/4), 0, m_origin.z - (m_height/4)), m_width/2, m_height/2);
        m_children.emplace_back(leftTopPatch);
        leftTopPatch->process();

    }

    void QuadTreePatch::createContent() {

        float planeLeft = -m_context.m_planeWidth / 2;
        float planeBottom = m_context.m_planeHeight / 2; // Positive, because this is the Z axis

        float patchUVLeft = planeLeft == m_leftEdge ? 0 : ((m_leftEdge - planeLeft) / m_context.m_planeWidth); // If the patch is on the left edge of the plane, avoid dividing by 0, U is 0
        float patchUVRight = (m_rightEdge - planeLeft) / m_context.m_planeWidth;
        float patchUVBottom = planeBottom == m_bottomEdge ? 0 : -((m_bottomEdge - planeBottom) / m_context.m_planeHeight); // Same as with planeLeft, avoid dividing by 0, V is 0
        float patchUVTop = -(m_topEdge - planeBottom) / m_context.m_planeHeight;

        m_context.m_quads.push_back({
            {
                {{m_leftEdge, 0, m_bottomEdge, 1}, {0, 1, 0}, {patchUVLeft, patchUVBottom}},
                {{m_rightEdge, 0, m_bottomEdge, 1}, {0, 1, 0}, {patchUVRight, patchUVBottom}},
                {{m_rightEdge, 0, m_topEdge, 1}, {0, 1, 0}, {patchUVRight, patchUVTop}},
                {{m_leftEdge, 0, m_topEdge, 1}, {0, 1, 0}, {patchUVLeft, patchUVTop}}
            },
            {0, 1, 2, 3},
            this,
            static_cast<unsigned int>(m_position)
        });

    }

    void QuadTreePatch::processEdges(PlaneQuad& quad) {

        auto leftNeighbour = findHorizontalNeighbour(HorizontalSide::Left);
        if (leftNeighbour != nullptr) {
            quad.m_leftSizeRatio = leftNeighbour->m_height / m_height;
        }

        auto bottomNeighbour = findVerticalNeighbour(VerticalSide::Bottom);
        if (bottomNeighbour != nullptr) {
            quad.m_bottomSizeRatio = bottomNeighbour->m_width / m_width;
        }

        auto rightNeighbour = findHorizontalNeighbour(HorizontalSide::Right);
        if (rightNeighbour != nullptr) {
            quad.m_rightSizeRatio = rightNeighbour->m_height / m_height;
        }

        auto topNeighbour = findVerticalNeighbour(VerticalSide::Top);
        if (topNeighbour != nullptr) {
            quad.m_topSizeRatio = topNeighbour->m_width / m_width;
        }

    }

    /*
     * To find a neighbour, we first ascend the ancestry tree until we find the common ancestor, recording the position of each node we visit (except the ancestor itself)
     * The common ancestor will be that node that is not on the same side as the edge we're looking for
     * In the case where we are on the left, and we're looking for the right neighbour, this would mean that we are the common ancestor
     * Once we've reached the common ancestor, we descend back down, retracing the steps backward and mirrored (either horizontally or vertically)
     */
    gaunlet::Core::Ref<QuadTreePatch> QuadTreePatch::findHorizontalNeighbour(HorizontalSide side) {

        // Discard the root-node case
        if (m_parent == nullptr) {
            return nullptr;
        }

        // Look for the common ancestor
        HorizontalSide oppositeSide = side == HorizontalSide::Left ? HorizontalSide::Right : HorizontalSide::Left;
        std::vector<PatchPosition> steps;
        auto commonAncestor = findFirstHorizontalSideAncestor(oppositeSide, steps);

        // If we haven't found such an ancestor, it means that we are on the edge of the plane
        if (commonAncestor == nullptr) {
            return nullptr;
        }

        // Now that we have an ancestor that is not on the same side, we get its horizontal neighbour
        auto neighbourAncestor = commonAncestor->getSibling(
            getHorizontalNeighbourPosition(commonAncestor->m_position)
        );

        // Now go back down the ancestry tree
        auto neighbour = neighbourAncestor->findDescendant(steps, true);

        return neighbour;

    }

    // Same as findHorizontalNeighbour, but vertically
    gaunlet::Core::Ref<QuadTreePatch> QuadTreePatch::findVerticalNeighbour(VerticalSide side) {

        // Discard the root-node case
        if (m_parent == nullptr) {
            return nullptr;
        }

        // Look for the common ancestor
        VerticalSide oppositeSide = side == VerticalSide::Bottom ? VerticalSide::Top : VerticalSide::Bottom;
        std::vector<PatchPosition> steps;
        auto commonAncestor = findFirstVerticalSideAncestor(oppositeSide, steps);

        // If we haven't found such an ancestor, it means that we are on the edge of the plane
        if (commonAncestor == nullptr) {
            return nullptr;
        }

        // Now that we have an ancestor that is not on the same side, we get its vertical neighbour
        auto neighbourAncestor = commonAncestor->getSibling(
            getVerticalNeighbourPosition(commonAncestor->m_position)
        );

        // Now go back down the ancestry tree
        auto neighbour = neighbourAncestor->findDescendant(steps, false);

        return neighbour;

    }

    // Find the first ancestor that is on the left/right, and return the steps taken (the position of each patch, up until the ancestor, excluded)
    gaunlet::Core::Ref<QuadTreePatch> QuadTreePatch::findFirstHorizontalSideAncestor(HorizontalSide side, std::vector<PatchPosition>& steps) {

        // The root node isn't a valid side-ancestor, because it has no siblings
        if (m_parent == nullptr) {
            return nullptr;
        }

        // If we are on the requested side, we're the ancestor
        if (getHorizontalSide() == side) {
            return shared_from_this();
        } else {
            steps.push_back(m_position);
            return m_parent->findFirstHorizontalSideAncestor(side, steps);
        }

    }

    gaunlet::Core::Ref<QuadTreePatch> QuadTreePatch::findFirstVerticalSideAncestor(VerticalSide side, std::vector<PatchPosition>& steps) {

        // The root node isn't a valid side-ancestor, because it has no siblings
        if (m_parent == nullptr) {
            return nullptr;
        }

        // If we are on the requested side, we're the ancestor
        if (getVerticalSide() == side) {
            return shared_from_this();
        } else { // Otherwise, record out position in the steps and ask our parent
            steps.push_back(m_position);
            return m_parent->findFirstVerticalSideAncestor(side, steps);
        }

    }

    // Find a descendant by retracing the steps taken to find the common ancestor, but backwards and mirrored
    gaunlet::Core::Ref<QuadTreePatch> QuadTreePatch::findDescendant(const std::vector<PatchPosition>& steps, bool horizontal) {

        gaunlet::Core::Ref<QuadTreePatch> descendant = shared_from_this();

        for (unsigned int i = steps.size(); i-- > 0;) {

            auto& step = steps[i];

            // If we've run out of levels, stop here, and we return a bigger quad
            if (descendant->m_children.empty()) {
                break;
            }

            auto oppositeStep = horizontal ? getHorizontalNeighbourPosition(step) : getVerticalNeighbourPosition(step);
            descendant = descendant->getChild(oppositeStep);

        }

        // If there are more levels to go, return the whatever child. They are all the same size
        if (!descendant->m_children.empty()) {
            descendant = descendant->m_children[0];
        }

        return descendant;

    }

    gaunlet::Core::Ref<QuadTreePatch> QuadTreePatch::getSibling(PatchPosition position) {
        return m_parent->getChild(position);
    }

    gaunlet::Core::Ref<QuadTreePatch> QuadTreePatch::getChild(PatchPosition position) {
        return m_children[static_cast<unsigned int>(position)];
    }

    HorizontalSide QuadTreePatch::getHorizontalSide() {
        if (m_position == PatchPosition::LeftBottom || m_position == PatchPosition::LeftTop) {
            return HorizontalSide::Left;
        } else {
            return HorizontalSide::Right;
        }
    }

    VerticalSide QuadTreePatch::getVerticalSide() {
        if (m_position == PatchPosition::LeftBottom || m_position == PatchPosition::RightBottom) {
            return VerticalSide::Bottom;
        } else {
            return VerticalSide::Top;
        }
    }

    PatchPosition QuadTreePatch::getHorizontalNeighbourPosition(PatchPosition position) {

        switch (position) {
            case PatchPosition::LeftBottom:     return PatchPosition::RightBottom;
            case PatchPosition::RightBottom:    return PatchPosition::LeftBottom;
            case PatchPosition::RightTop:       return PatchPosition::LeftTop;
            case PatchPosition::LeftTop:        return PatchPosition::RightTop;
        }

    }

    PatchPosition QuadTreePatch::getVerticalNeighbourPosition(PatchPosition position) {

        switch (position) {
            case PatchPosition::LeftBottom:     return PatchPosition::LeftTop;
            case PatchPosition::RightBottom:    return PatchPosition::RightTop;
            case PatchPosition::RightTop:       return PatchPosition::RightBottom;
            case PatchPosition::LeftTop:        return PatchPosition::LeftBottom;
        }

    }

}