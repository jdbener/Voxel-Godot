#include "Chunk.h"

#include <cmath>
#include <map>

/*------------------------------------------------------------------------------
        VoxelInstance
------------------------------------------------------------------------------*/

// Copy constructor
VoxelInstance::VoxelInstance(VoxelInstance& origin){
    data = new BlockData();
    *data = *origin.data;

    level = origin.level;
    center = origin.center;
    subVoxels = origin.subVoxels;
    parent = origin.parent;
}

// Move constructor
VoxelInstance::VoxelInstance(VoxelInstance&& origin){
    data = new BlockData();
    *data = *origin.data;

    level = origin.level;
    center = origin.center;
    subVoxels = origin.subVoxels;
    parent = origin.parent;
}

// Function which recursiveley converts an array of blockIDs into an octree
void VoxelInstance::init(int blocks[], int level /*= SUBCHUNK_LEVELS*/, bool originalCall /*= true*/){
    this->level = level; // Mark which sublevel this instance is

    // If there is data to initalize and we aren't dealing with leaf nodes...
    if(blocks && level > 0){
            subVoxels = new VoxelInstance [8]; // Create the next sublevels

        // Determine how many blocks are in the source array at this level
        int numLeft = pow(2, level);
        numLeft *= numLeft * numLeft;

        // Distribute the blocks over the child sublevels
        for(int i = 0; i < 8; i++){
            subVoxels[i].parent = this; // Mark this sublevel as the parent of the child sublevels
            subVoxels[i].init(&blocks[i * (numLeft / 8)], level - 1, false);
        }
    // If we are dealing with leaf nodes...
    } else
        data->blockID = blocks[0];

    // If this was the original call recalculate
    if(originalCall)
        recalculate();
}

// Function which merges sublevels containing all of the same blockID into the same level
bool VoxelInstance::prune(){
    // Variable which ensures that small data changes not large enouph to change the mathematical
    // mode of a sublevel don't get pruned
    bool canPrune = true;
    if(level > 1 && subVoxels)
        for(size_t i = 0; i < 8; i++)
            if(!subVoxels[i].prune())
                canPrune = false;

    uint count = 0;	// Number of times the most common blockID appears in the sublevels
    if(subVoxels){ // Make sure there are sublevels before finding the mode of the sublevels
        std::map<uint, uint> m; // Map used to sort blockIDs
        // Store the blockIDs sorted by occurence in a map
        for(size_t i = 0; i < 8; i++) {
            uint key = subVoxels[i].data->blockID;
            std::map<uint, uint>::iterator it = m.find(key);
            if(it == m.end())
                m.insert(std::make_pair(key, 1));
            else
                it->second++;
        }
        // Find the most common blockID in the sublevels and store it as this level's blockID
        for(auto& it: m)
            if(it.second > count) {
                count = it.second;
                data->blockID = it.first;
            }
    } else
        return true; // If we have already pruned this branch we are safe to prune higher

    // If the sublevels all have the same blockID, they can be removed
    if(count == 8 && canPrune){
        delete [] subVoxels;
        subVoxels = nullptr;
        return true; // We are safe to prune higher up
    }
    return false; // We are not safe to prune higher up
}

// Function which takes a pruned tree and rebuilds the lower levels of the tree down to the block level
// The purpose of this function is to rebuild a branch of the tree when we need to modify a blockID in that branch
void VoxelInstance::unprune(bool originalCall /*= true*/){
    // If we don't have children and we aren't at the block level
    if(level > 0 && !subVoxels){
        subVoxels = new VoxelInstance [8]; // Create the next sublevels

        // Pass down the blockID to all the new children and unprune them
        for(int i = 0; i < 8; i++){
            subVoxels[i].parent = this;
            //subVoxels[i].blockID = blockID;
            subVoxels[i].level = level - 1;
            *subVoxels[i].data = *data;
            subVoxels[i].unprune(false);
        }
    // If we do have children and we aren't at the block level
    } else if (level > 0)
        // Unprune the children
        for(int i = 0; i < 8; i++)
            subVoxels[i].unprune(false);

    // If this is the original function call... recalculate
    if(originalCall)
        recalculate();
}

// Function which given an arbitrary point in 3D space within the voxel
// finds the subvoxel of the requested <lvl> which contains the point
VoxelInstance* VoxelInstance::find(int lvl, Vector3& position){
    // If we are at the desired level... return a reference to this node
    if(lvl == level){
        return this;
    }

    // If this node has children...
    if(subVoxels)
        for(int i = 0; i < 8; i++)
            // Check if each child encloses the search position...
            if(subVoxels[i].within(position))
                // If it does recursively search into that child
                return subVoxels[i].find(lvl, position);
    // If we are at a leaf node... return a reference to this node
    return this;
}

// Function which determines if an arbitrary point in space is within this voxel
bool VoxelInstance::within(Vector3& position){
    // Calculate the "radius" of the cube based on its level
    int bounds = pow(2, level - 1);
    if(bounds < 1) bounds = 1;
    // Check if the arbitrary point is within those bounds
    return !(position.x <= center.x - bounds || position.x >= center.x + bounds
        || position.y <= center.y - bounds || position.y >= center.y + bounds
        || position.z <= center.z - bounds || position.z >= center.z + bounds);
}

// Function which recursively calculates the center of all of the sub voxels
void VoxelInstance::calculateCenters(){
    if(subVoxels){
        // TODO: check if there is a bug here
        #warning check if there is a bug here
        float change = .5 * pow(2, level-1);

        subVoxels[0].center = center + Vector3 {change, change, change};
        subVoxels[1].center = center + Vector3 {change, change, -change};
        subVoxels[2].center = center + Vector3 {-change, change, -change};
        subVoxels[3].center = center + Vector3 {-change, change, change};
        subVoxels[4].center = center + Vector3 {change, -change, change};
        subVoxels[5].center = center + Vector3 {change, -change, -change};
        subVoxels[6].center = center + Vector3 {-change, -change, -change};
        subVoxels[7].center = center + Vector3 {-change, -change, change};

        for(int i = 0; i < 8; i++)
            subVoxels[i].calculateCenters();
    }
}


// Debug functions
int VoxelInstance::count(){
    int count = 0;
    if(subVoxels){
        for(size_t i = 0; i < 8; i++)
            count += subVoxels[i].count();
        return count + 1;
    }
    return 1;
}

String copy(char what, int times){
    String out = "";
    for(int i = 0; i < times; i++)
        out += what;
    return out;
}

String VoxelInstance::dump(){
    String out = copy('\t', SUBCHUNK_LEVELS - level) + String::num_int64(level) + " - " + String::num_int64(data->blockID) + " - " + center + '\n';
    if(subVoxels)
        for(size_t i = 0; i < 8; i++)
            out += subVoxels[i].dump();
    return out;
}
