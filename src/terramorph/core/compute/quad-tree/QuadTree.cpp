#include "terramorph/core/compute/quad-tree/QuadTree.h"

#include <utility>

namespace terramorph::Core {

    std::vector<Quad>QuadTreePatch::compute(SubdivisionFunctor subdivisionFunctor, VertexFunctor vertexFunctor) {

        Context context(std::move(subdivisionFunctor), std::move(vertexFunctor));

        // Create a root patch
        auto rootNode = gaunlet::Core::CreateRef<QuadTreePatch>(nullptr, PatchPosition::Root, context, glm::vec3(0, 0, 0), 2.0f, 2.0f);

        // Process the patch, recursively subdividing, and creating the quads
        rootNode->process();

        // Re-process each quad, finding its size ratios with its neighbours
        for (auto& quad : context.m_quads) {
            quad.m_patch->processEdges(quad);
        }

        return context.m_quads;

    }

    QuadTreePatch::QuadTreePatch(QuadTreePatch* parent, PatchPosition position, Context &context, glm::vec2 origin, float width, float height)
        : m_parent(parent), m_position(position), m_context(context), m_origin(origin), m_width(width), m_height(height) {
        computeDimensions();
    }

    void QuadTreePatch::computeDimensions() {

        m_leftEdge = m_origin.x - (m_width / 2.0f);
        m_rightEdge = m_origin.x + (m_width / 2.0f);
        m_bottomEdge = m_origin.y - (m_height / 2.0f);
        m_topEdge = m_origin.y + (m_height / 2.0f);

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

        float leftU = (m_leftEdge + 1.0f) / 2.0f;
        float rightU = (m_rightEdge + 1.0f) / 2.0f;
        float bottomV = (m_bottomEdge + 1.0f) / 2.0f;
        float topV = (m_topEdge + 1.0f) / 2.0f;

        return m_context.m_subdivisionFunctor(
            leftU,
            rightU,
            bottomV,
            topV
        );

    }

    void QuadTreePatch::subdivide() {

        auto leftBottomPatch = gaunlet::Core::CreateRef<QuadTreePatch>(this, PatchPosition::LeftBottom, m_context, glm::vec2(m_origin.x - (m_width/4), m_origin.y - (m_height/4)), m_width/2, m_height/2);
        m_children.emplace_back(leftBottomPatch);
        leftBottomPatch->process();

        auto rightBottomPatch = gaunlet::Core::CreateRef<QuadTreePatch>(this, PatchPosition::RightBottom, m_context, glm::vec2(m_origin.x + (m_width/4), m_origin.y - (m_height/4)), m_width/2, m_height/2);
        m_children.emplace_back(rightBottomPatch);
        rightBottomPatch->process();

        auto rightTopPatch = gaunlet::Core::CreateRef<QuadTreePatch>(this, PatchPosition::RightTop, m_context, glm::vec2(m_origin.x + (m_width/4), m_origin.y + (m_height/4)), m_width/2, m_height/2);
        m_children.emplace_back(rightTopPatch);
        rightTopPatch->process();

        auto leftTopPatch = gaunlet::Core::CreateRef<QuadTreePatch>(this, PatchPosition::LeftTop, m_context, glm::vec2(m_origin.x - (m_width/4), m_origin.y + (m_height/4)), m_width/2, m_height/2);
        m_children.emplace_back(leftTopPatch);
        leftTopPatch->process();

    }

    void QuadTreePatch::createContent() {

        m_context.m_quads.push_back({
            {
                m_context.m_vertexFunctor(m_leftEdge, m_bottomEdge),
                m_context.m_vertexFunctor(m_rightEdge, m_bottomEdge),
                m_context.m_vertexFunctor(m_rightEdge, m_topEdge),
                m_context.m_vertexFunctor(m_leftEdge, m_topEdge)
            },
            {0, 1, 2, 3},
            this,
            static_cast<unsigned int>(m_position)
        });

    }

    void QuadTreePatch::processEdges(Quad &quad) {

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
     * In the case where we are on the left (relative to our parent), and we're looking for the right neighbour, this would mean that we are the common ancestor
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
    gaunlet::Core::Ref<QuadTreePatch> QuadTreePatch::findFirstHorizontalSideAncestor(HorizontalSide side, std::vector<PatchPosition> &steps) {

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

    gaunlet::Core::Ref<QuadTreePatch>QuadTreePatch::findFirstVerticalSideAncestor(VerticalSide side, std::vector<PatchPosition> &steps) {

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

    gaunlet::Core::Ref<QuadTreePatch>QuadTreePatch::findDescendant(const std::vector<PatchPosition> &steps, bool horizontal) {

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

    QuadTreePatch::HorizontalSide QuadTreePatch::getHorizontalSide() {

        if (m_position == PatchPosition::LeftBottom || m_position == PatchPosition::LeftTop) {
            return HorizontalSide::Left;
        } else {
            return HorizontalSide::Right;
        }

    }

    QuadTreePatch::VerticalSide QuadTreePatch::getVerticalSide() {

        if (m_position == PatchPosition::LeftBottom || m_position == PatchPosition::RightBottom) {
            return VerticalSide::Bottom;
        } else {
            return VerticalSide::Top;
        }

    }

    QuadTreePatch::PatchPosition QuadTreePatch::getHorizontalNeighbourPosition(PatchPosition position) {

        switch (position) {
            case PatchPosition::LeftBottom:     return PatchPosition::RightBottom;
            case PatchPosition::RightBottom:    return PatchPosition::LeftBottom;
            case PatchPosition::RightTop:       return PatchPosition::LeftTop;
            case PatchPosition::LeftTop:        return PatchPosition::RightTop;
        }

    }

    QuadTreePatch::PatchPosition QuadTreePatch::getVerticalNeighbourPosition(PatchPosition position) {

        switch (position) {
            case PatchPosition::LeftBottom:     return PatchPosition::LeftTop;
            case PatchPosition::RightBottom:    return PatchPosition::RightTop;
            case PatchPosition::RightTop:       return PatchPosition::RightBottom;
            case PatchPosition::LeftTop:        return PatchPosition::LeftBottom;
        }

    }

}