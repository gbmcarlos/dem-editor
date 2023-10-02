#pragma once

#include <utility>

#include "terramorph/pch.h"

namespace terramorph::Core {

    class QuadTreePatch;

    // This is the information that is ultimately sent to be rendered
    struct Quad {

        std::vector<gaunlet::Graphics::Vertex> m_vertices;
        std::vector<unsigned int> m_indices;
        QuadTreePatch* m_patch;
        unsigned int m_position;
        float m_leftSizeRatio = 0.0f;
        float m_bottomSizeRatio = 0.0f;
        float m_rightSizeRatio = 0.0f;
        float m_topSizeRatio = 0.0f;

    };

    class QuadTreePatch : public std::enable_shared_from_this<QuadTreePatch> {

        using VertexFunctor = std::function<gaunlet::Graphics::Vertex(float, float)>;
        using SubdivisionFunctor = std::function<bool(float, float, float, float)>;

    private:

        // This is just a collection of data that is passed around
        struct Context {

            Context(SubdivisionFunctor subdivisionFunctor, VertexFunctor vertexFunctor)
                : m_subdivisionFunctor(std::move(subdivisionFunctor)), m_vertexFunctor(std::move(vertexFunctor)) {}

            std::vector<Quad> m_quads;
            SubdivisionFunctor m_subdivisionFunctor;
            VertexFunctor m_vertexFunctor;

        };

        enum class PatchPosition {
            LeftBottom = 0, RightBottom = 1,
            RightTop = 2, LeftTop = 3,
            Root = 4
        };

        enum class HorizontalSide {
            Left, Right
        };

        enum class VerticalSide {
            Bottom, Top
        };

    public:

        static std::vector<Quad> compute(SubdivisionFunctor subdivisionFunctor, VertexFunctor vertexFunctor);

        QuadTreePatch(QuadTreePatch* parent, PatchPosition position, Context& context, glm::vec2 origin, float width, float height);

    private:

        QuadTreePatch* m_parent = nullptr;
        PatchPosition m_position;
        Context& m_context;
        std::vector<gaunlet::Core::Ref<QuadTreePatch>> m_children = {};
        glm::vec2 m_origin;
        float m_width;
        float m_height;
        float m_leftEdge;
        float m_rightEdge;
        float m_bottomEdge;
        float m_topEdge;

        void computeDimensions();

        void process();

        bool requiresSubdivision();

        void subdivide();

        void createContent();

        void processEdges(Quad& quad);

        HorizontalSide getHorizontalSide();

        VerticalSide getVerticalSide();

        gaunlet::Core::Ref<QuadTreePatch> findHorizontalNeighbour(HorizontalSide side);

        gaunlet::Core::Ref<QuadTreePatch> findVerticalNeighbour(VerticalSide side);

        gaunlet::Core::Ref<QuadTreePatch> findFirstHorizontalSideAncestor(HorizontalSide side, std::vector<PatchPosition>& steps);

        gaunlet::Core::Ref<QuadTreePatch> findFirstVerticalSideAncestor(VerticalSide side, std::vector<PatchPosition>& steps);

        gaunlet::Core::Ref<QuadTreePatch> findDescendant(const std::vector<PatchPosition>& steps, bool horizontal);

        gaunlet::Core::Ref<QuadTreePatch> getSibling(PatchPosition position);

        gaunlet::Core::Ref<QuadTreePatch> getChild(PatchPosition position);

        PatchPosition getHorizontalNeighbourPosition(PatchPosition position);

        PatchPosition getVerticalNeighbourPosition(PatchPosition position);

    };

}