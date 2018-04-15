#pragma once

#include <algorithm>
#include <ctime>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/vec3.hpp>
#include <iterator>
#include <memory>
#include <random>
#include <stack>
#include <utility>
#include <vector>

#include <iostream>

#include "../mathutil.h"
#include "bbox.h"
#include "scene.h"

using std::cout;
using std::endl;
using std::unique_ptr;
using std::ostream;
extern bool debugMode;

ostream& operator<<(ostream& os, const glm::dvec3);
ostream& operator<<(ostream& os, const BoundingBox& b);

// Note: you can put kd-tree here
template <typename T>
class KdNode {
    public:
    const KdNode& getParent() const { return parent; };
    const KdNode*& getChildren() const { return children; };
    const KdNode* const getLeftChild() const { return children[0]; };
    const KdNode* const getRightChild() const { return children[1]; };
    KdNode* getBuildLeftChild() { return children[0]; };
    KdNode* getBuildRightChild() { return children[1]; };
    const BoundingBox& getExtent() const { return extent; };
    const glm::dvec3& getSplitPt() const { return splitPt; };
    const std::vector<T*>& getElems() const { return elems; };
    int nodeSize() const { return elems.size(); };

    bool isLeaf() const
    {
        return children[0] == nullptr && children[1] == nullptr;
    };

    KdNode(KdNode* p, const BoundingBox& b, std::vector<T*> inp)
            : parent(p),
              extent(b),
              children{nullptr, nullptr},
              splitPt(glm::dvec3(0.0))
    {
        elems = std::vector<T*>(inp.begin(), inp.end());
    }

    /* Split a node in twain. This operation forms the core of the KdTree
     * construction. First, find the longest axis to split along. Then, find a
     * plane close to median which does not bisect objects, unless the resulting
     * split is too biased, and split the tree there.
     *
     * Hey idiot, maxDepth should be handled by the Tree, not the nodes! */
    void splitNode()
    {
        if (elems.size() <= 1) {
            return; // Cowardly refuse to split singleton tree
        }

        int splitAxis; // Which axis to split along. 0=x, y=1, z=2
        glm::dvec3 deltas(extent.getMax()[0] - extent.getMin()[0],
                          extent.getMax()[1] - extent.getMin()[1],
                          extent.getMax()[2] - extent.getMin()[2]);

        if (deltas[0] >= deltas[1] && deltas[0] >= deltas[2]) {
            splitAxis = 0;
        } else if (deltas[1] >= deltas[0] && deltas[1] >= deltas[2]) {
            splitAxis = 1;
        } else {
            splitAxis = 2;
            assert(deltas[2] >= deltas[0] && deltas[2] >= deltas[1]);
        }

        /* We seek to split the elements roughly evenly into two partitions, so
         * that the number of elements on one side of the split is the same as
         * on the other, while splitting as few elements as possible. The
         * strategy is to find a representative median object and split along
         * one side of it (for simplicity, the larger side). How do we find this
         * median?
         *
         * If there are few enough elements in the class, we simply sort the
         * elements and choose the median. If there are more than this, we
         * sample some fraction of the elements and choose the median of the
         * sampled list. */
        T* medianElement;
        size_t sz;
        bool sample = false;

        if (elems.size() > 1000) {
            sz = (size_t)(elems.size() * 0.10);
            sample = true;
        } else if (elems.size() > 200) {
            sz = (size_t)(elems.size() * 0.20);
            sample = true;
        } else {
            std::sort(elems.begin(), elems.end(), GeometrySorter(splitAxis));
        }

        if (sample) {
            // Select sample elements to be sorted
            std::vector<T*> samples;
            MathUtil::FYShuffle randSrc(sz);
            for (size_t j = 0; j < sz; j++) {
                samples.push_back(elems[randSrc.next()]);
            }
            // Sort samples
            std::sort(samples.begin(), samples.end(),
                      GeometrySorter(splitAxis));
            // Find median element
            medianElement = (samples[samples.size() / 2]);
        } else {
            // Median element is just the median element of the middle
            medianElement = (elems[elems.size() / 2]);
        }

        this->splitPt[splitAxis] =
                medianElement->getBoundingBox().getMax()[splitAxis];

        // Generate child node data
        std::vector<T*> leftData;
        std::vector<T*> rightData;

        // Compute extents of left and right children
        glm::dvec3 leftEnd = extent.getMax();
        leftEnd[splitAxis] = splitPt[splitAxis];
        glm::dvec3 rightBegin = extent.getMin();
        rightBegin[splitAxis] = splitPt[splitAxis];
        const BoundingBox leftBox(extent.getMin(), leftEnd);
        const BoundingBox rightBox(rightBegin, extent.getMax());

        // Silly initial estimate: if bounding boxes of elements intersect
        // w/ extent of child, assume that the element is in that box
        for (const auto& elem : elems) {
            if (elem->getBoundingBox().intersects(leftBox)) {
                leftData.push_back(elem);
            }
            if (elem->getBoundingBox().intersects(rightBox)) {
                rightData.push_back(elem);
            }
        }

/*
        cout << "BBox is " << extent << endl;
        cout << "Left Box is " << leftBox << endl;
        cout << "Right Box is " << rightBox << endl;
*/

        children[0] = new KdNode(this, leftBox, leftData);
        children[1] = new KdNode(this, rightBox, rightData);
    }

    /* Does the given ray intersect with the cell? If so, place the two times of
     * intersection into tmin and tmax. */
    bool intersect(const ray& r, double& tmin, double& tmax) const
    {
        return extent.intersect(r, tmin, tmax);
    }
    /* Does the given ray intersect with the cell's elements? Return the
       earliest collision detected in this cell.
       NB: if you call this on non-leaf cells, you're gonna have a bad time */
    bool intersectElem(const ray& r, isect& i) const
    {
        isect earliest, testIsect;
        bool have_one = false;

        ray r2(r);

        for (const auto& geom : elems) {
            if (geom->intersect(r2, testIsect)) {
                if (!have_one) {
                    have_one = true;
                    earliest = testIsect;
                } else {
                    if (testIsect.getT() < earliest.getT()) {
                        earliest = testIsect;
                    }
                }
            }
        }

        if (have_one) {
            i = earliest;
        }
        return have_one;
    }

    private:
    /* Tree metadata: convention holds that left child is children[0]. */
    KdNode* children[2];
    KdNode* parent;

    /* Geometric data. Consist of a bounding box specifying the spatial extent
     * of this node and a vector specifying where the box is split. The vector
     * should have two zero elements--i.e. l2Norm(splitPt) == infNorm(splitPt),
     * or be all zero to indicate an unsplit cell. */
    glm::dvec3 splitPt;
    BoundingBox extent;

    // Tree datadata
    std::vector<T*> elems;

    void setElems(std::vector<T*> inp)
    {
        for (const auto& e : inp) {
            elems.push_back(e);
        }
    }
};

template <typename T>
class KdTree {
    public:
    using KDN = KdNode<T>;
    KdTree(const BoundingBox& bb, const std::vector<T*>& elems)
            : isBuilt(false), numNodes(1)
    {
        root = std::make_unique<KDN>(nullptr, bb, elems);
    }

    void build(int maxDepth, int targSize)
    {
        std::clock_t start = std::clock();

        if (isBuilt) {
            assert("Cowardly refusing to rebuild a KDTree.");
        }

        /* Build the tree in a DFS-fashion to improve cache locality. */
        std::stack<std::pair<KDN*, int>> st;
        auto firstVisit = std::make_pair(root.get(), 0);
        st.push(firstVisit);
        while (!st.empty()) {
            auto targ = st.top();
            st.pop();
            KDN* node = targ.first;
            int depth = targ.second;
            if (depth > maxDepth || node->nodeSize() < targSize) {
                continue;
            }

            node->splitNode();
            st.push(std::make_pair(node->getBuildLeftChild(), depth + 1));
            st.push(std::make_pair(node->getBuildRightChild(), depth + 1));
            numNodes += 2;
        }
        double duration = (std::clock() - start) / (double)CLOCKS_PER_SEC;
        std::cout << "KDTree Build took " << duration << " seconds and has "
                  << numNodes << " nodes." << std::endl;

        isBuilt = true;
    }

    bool intersect(const ray& r, isect& i) const
    {
        // Walk the tree my captain lies, fallen cold and...wait, wrong context.
        double tmin = 0, tmax = 0;

        // If the main BB does not intersect the ray, no intersection can occur
        if (!root->intersect(r, tmin, tmax)) {
            return false;
        }

        std::stack<const KDN*> toVisit;
        toVisit.push(const_cast<const KDN*>(root.get()));

        // Use earliest time of a detected collision to prune search: if
        // tmin > earliestTime, no need to visit that cell.
        double earliestTime = std::numeric_limits<double>::max();
        isect earliestI;
        bool have_one = false;

        while (!toVisit.empty()) {
            const KDN* visit = toVisit.top();
            toVisit.pop(); // Emulate classic pop

            if (visit->isLeaf()) {
                if(debugMode) cout << "Is leaf. Test elements." << endl;
                isect tmp;
                if (visit->intersectElem(r, tmp)) {
                    if (tmp.getT() < earliestTime) {
                        earliestI = tmp;
                        earliestTime = tmp.getT();
                        have_one = true;
                        assert(earliestTime > 0);
                    }
                }
            } else { // Do DFS visits with pruning
                    if(debugMode) cout << "Not a leaf. Test children." << endl;
                    isect dummy;

                    const KDN* lChild = visit->getLeftChild();
                    const KDN* rChild = visit->getRightChild();

                    bool added = false;

                    if(!visit->intersect(r,tmin,tmax)){
                        // The fuck we doing here?
                        continue;
                    }

                    if(lChild->intersect(r,tmin,tmax)){
                        if(tmin < earliestTime){
                            toVisit.push(lChild);
                        }
                    }

                    if(rChild->intersect(r,tmin,tmax)){
                        if(tmin < earliestTime){
                            toVisit.push(rChild);
                        }
                    }
                }
            }

        i = earliestI;
        return have_one;
    }

    private:
    unique_ptr<KdNode<T>> root;
    bool isBuilt;
    int numNodes;
};