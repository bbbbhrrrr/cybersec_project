#include "../common/sm3_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/**
 * SM3-based Merkle Tree Implementation
 * 
 * 基于RFC 6962标准的Merkle树实现
 * 支持10万叶子节点的高效构建和证明生成
 */

// Merkle树节点结构
typedef struct merkle_node {
    uint8_t hash[SM3_DIGEST_SIZE];
    struct merkle_node *left;
    struct merkle_node *right;
    int is_leaf;
    size_t index;  // 叶子节点的索引
} merkle_node_t;

// Merkle树结构
typedef struct {
    merkle_node_t *root;
    merkle_node_t **leaves;  // 叶子节点数组
    size_t leaf_count;
    size_t capacity;
    int height;
} merkle_tree_t;

// Merkle证明结构
typedef struct {
    uint8_t (*path_hashes)[SM3_DIGEST_SIZE];  // 证明路径上的哈希值
    int *directions;                          // 0=左兄弟, 1=右兄弟
    size_t path_length;
    size_t leaf_index;
} merkle_proof_t;

// 不存在性证明结构
typedef struct {
    merkle_proof_t left_proof;   // 左边界证明
    merkle_proof_t right_proof;  // 右边界证明
    size_t target_index;         // 目标索引
} merkle_nonexistence_proof_t;

/**
 * 根据RFC 6962计算叶子节点哈希
 * MTH({d(0), d(1), ..., d(n-1)}) = 
 *   H(0x00 || data) for single leaf
 */
static void compute_leaf_hash(const uint8_t *data, size_t len, uint8_t hash[SM3_DIGEST_SIZE]) {
    sm3_ctx_t ctx;
    sm3_init(&ctx);
    
    // 添加叶子前缀 0x00
    uint8_t leaf_prefix = 0x00;
    sm3_update(&ctx, &leaf_prefix, 1);
    sm3_update(&ctx, data, len);
    
    sm3_final(&ctx, hash);
}

/**
 * 根据RFC 6962计算内部节点哈希
 * MTH(D[k:k+2^i]) = H(0x01 || MTH(D[k:k+2^(i-1)]) || MTH(D[k+2^(i-1):k+2^i]))
 */
static void compute_internal_hash(const uint8_t left_hash[SM3_DIGEST_SIZE], 
                                  const uint8_t right_hash[SM3_DIGEST_SIZE],
                                  uint8_t hash[SM3_DIGEST_SIZE]) {
    sm3_ctx_t ctx;
    sm3_init(&ctx);
    
    // 添加内部节点前缀 0x01
    uint8_t internal_prefix = 0x01;
    sm3_update(&ctx, &internal_prefix, 1);
    sm3_update(&ctx, left_hash, SM3_DIGEST_SIZE);
    sm3_update(&ctx, right_hash, SM3_DIGEST_SIZE);
    
    sm3_final(&ctx, hash);
}

/**
 * 创建新的Merkle树
 */
merkle_tree_t* merkle_tree_create(size_t initial_capacity) {
    merkle_tree_t *tree = malloc(sizeof(merkle_tree_t));
    if (!tree) return NULL;
    
    tree->leaves = malloc(sizeof(merkle_node_t*) * initial_capacity);
    if (!tree->leaves) {
        free(tree);
        return NULL;
    }
    
    tree->root = NULL;
    tree->leaf_count = 0;
    tree->capacity = initial_capacity;
    tree->height = 0;
    
    return tree;
}

/**
 * 创建叶子节点
 */
static merkle_node_t* create_leaf_node(const uint8_t *data, size_t len, size_t index) {
    merkle_node_t *node = malloc(sizeof(merkle_node_t));
    if (!node) return NULL;
    
    compute_leaf_hash(data, len, node->hash);
    node->left = NULL;
    node->right = NULL;
    node->is_leaf = 1;
    node->index = index;
    
    return node;
}

/**
 * 创建内部节点
 */
static merkle_node_t* create_internal_node(merkle_node_t *left, merkle_node_t *right) {
    merkle_node_t *node = malloc(sizeof(merkle_node_t));
    if (!node) return NULL;
    
    compute_internal_hash(left->hash, right->hash, node->hash);
    node->left = left;
    node->right = right;
    node->is_leaf = 0;
    node->index = 0; // 内部节点不需要索引
    
    return node;
}

/**
 * 添加叶子节点到Merkle树
 */
int merkle_tree_add_leaf(merkle_tree_t *tree, const uint8_t *data, size_t len) {
    if (tree->leaf_count >= tree->capacity) {
        // 扩容
        size_t new_capacity = tree->capacity * 2;
        merkle_node_t **new_leaves = realloc(tree->leaves, 
                                            sizeof(merkle_node_t*) * new_capacity);
        if (!new_leaves) return 0;
        
        tree->leaves = new_leaves;
        tree->capacity = new_capacity;
    }
    
    merkle_node_t *leaf = create_leaf_node(data, len, tree->leaf_count);
    if (!leaf) return 0;
    
    tree->leaves[tree->leaf_count] = leaf;
    tree->leaf_count++;
    
    return 1;
}

/**
 * 递归构建Merkle树
 */
static merkle_node_t* build_tree_recursive(merkle_node_t **nodes, size_t count) {
    if (count == 1) {
        return nodes[0];
    }
    
    size_t next_level_count = (count + 1) / 2;
    merkle_node_t **next_level = malloc(sizeof(merkle_node_t*) * next_level_count);
    if (!next_level) return NULL;
    
    for (size_t i = 0; i < next_level_count; i++) {
        size_t left_idx = i * 2;
        size_t right_idx = left_idx + 1;
        
        if (right_idx < count) {
            // 有左右子节点
            next_level[i] = create_internal_node(nodes[left_idx], nodes[right_idx]);
        } else {
            // 只有左子节点，直接提升
            next_level[i] = nodes[left_idx];
        }
        
        if (!next_level[i]) {
            free(next_level);
            return NULL;
        }
    }
    
    merkle_node_t *result = build_tree_recursive(next_level, next_level_count);
    free(next_level);
    return result;
}

/**
 * 完成Merkle树构建
 */
int merkle_tree_finalize(merkle_tree_t *tree) {
    if (tree->leaf_count == 0) {
        return 0;
    }
    
    // 计算树的高度
    tree->height = (int)ceil(log2(tree->leaf_count));
    
    // 构建树
    tree->root = build_tree_recursive(tree->leaves, tree->leaf_count);
    
    return (tree->root != NULL);
}

/**
 * 生成存在性证明
 */
merkle_proof_t* merkle_tree_generate_proof(merkle_tree_t *tree, size_t leaf_index) {
    if (leaf_index >= tree->leaf_count || !tree->root) {
        return NULL;
    }
    
    merkle_proof_t *proof = malloc(sizeof(merkle_proof_t));
    if (!proof) return NULL;
    
    proof->leaf_index = leaf_index;
    proof->path_length = tree->height;
    proof->path_hashes = malloc(sizeof(uint8_t[SM3_DIGEST_SIZE]) * proof->path_length);
    proof->directions = malloc(sizeof(int) * proof->path_length);
    
    if (!proof->path_hashes || !proof->directions) {
        free(proof->path_hashes);
        free(proof->directions);
        free(proof);
        return NULL;
    }
    
    // 从叶子节点向上构建证明路径
    size_t current_index = leaf_index;
    size_t current_level_size = tree->leaf_count;
    merkle_node_t **current_level = malloc(sizeof(merkle_node_t*) * current_level_size);
    memcpy(current_level, tree->leaves, sizeof(merkle_node_t*) * current_level_size);
    
    for (int level = 0; level < proof->path_length; level++) {
        size_t sibling_index;
        
        if (current_index % 2 == 0) {
            // 当前节点是左子节点
            sibling_index = current_index + 1;
            proof->directions[level] = 1; // 兄弟节点在右边
        } else {
            // 当前节点是右子节点
            sibling_index = current_index - 1;
            proof->directions[level] = 0; // 兄弟节点在左边
        }
        
        if (sibling_index < current_level_size) {
            memcpy(proof->path_hashes[level], current_level[sibling_index]->hash, SM3_DIGEST_SIZE);
        } else {
            // 没有兄弟节点，使用当前节点的哈希
            memcpy(proof->path_hashes[level], current_level[current_index]->hash, SM3_DIGEST_SIZE);
        }
        
        // 移动到下一层
        current_index /= 2;
        size_t next_level_size = (current_level_size + 1) / 2;
        
        if (level < proof->path_length - 1) {
            merkle_node_t **next_level = malloc(sizeof(merkle_node_t*) * next_level_size);
            for (size_t i = 0; i < next_level_size; i++) {
                size_t left_idx = i * 2;
                size_t right_idx = left_idx + 1;
                
                if (right_idx < current_level_size) {
                    next_level[i] = create_internal_node(current_level[left_idx], current_level[right_idx]);
                } else {
                    next_level[i] = current_level[left_idx];
                }
            }
            
            free(current_level);
            current_level = next_level;
            current_level_size = next_level_size;
        }
    }
    
    free(current_level);
    return proof;
}

/**
 * 验证存在性证明
 */
int merkle_proof_verify(merkle_proof_t *proof, const uint8_t *leaf_data, size_t leaf_len,
                       const uint8_t root_hash[SM3_DIGEST_SIZE]) {
    if (!proof || !leaf_data || !root_hash) {
        return 0;
    }
    
    // 计算叶子节点哈希
    uint8_t current_hash[SM3_DIGEST_SIZE];
    compute_leaf_hash(leaf_data, leaf_len, current_hash);
    
    // 沿着证明路径计算根哈希
    for (size_t i = 0; i < proof->path_length; i++) {
        uint8_t combined_hash[SM3_DIGEST_SIZE];
        
        if (proof->directions[i] == 0) {
            // 兄弟节点在左边
            compute_internal_hash(proof->path_hashes[i], current_hash, combined_hash);
        } else {
            // 兄弟节点在右边
            compute_internal_hash(current_hash, proof->path_hashes[i], combined_hash);
        }
        
        memcpy(current_hash, combined_hash, SM3_DIGEST_SIZE);
    }
    
    // 比较计算出的根哈希和给定的根哈希
    return memcmp(current_hash, root_hash, SM3_DIGEST_SIZE) == 0;
}

/**
 * 生成不存在性证明
 * 证明索引target_index处不存在叶子节点
 */
merkle_nonexistence_proof_t* merkle_tree_generate_nonexistence_proof(merkle_tree_t *tree, size_t target_index) {
    if (target_index < tree->leaf_count) {
        // 目标索引实际存在，无法生成不存在性证明
        return NULL;
    }
    
    merkle_nonexistence_proof_t *proof = malloc(sizeof(merkle_nonexistence_proof_t));
    if (!proof) return NULL;
    
    proof->target_index = target_index;
    
    // 找到左边界（最大的索引 < target_index）
    size_t left_boundary = (target_index > 0) ? target_index - 1 : 0;
    while (left_boundary > 0 && left_boundary >= tree->leaf_count) {
        left_boundary--;
    }
    
    // 找到右边界（最小的索引 > target_index）
    size_t right_boundary = target_index + 1;
    while (right_boundary < tree->leaf_count) {
        right_boundary++;
    }
    if (right_boundary >= tree->leaf_count) {
        right_boundary = tree->leaf_count - 1;
    }
    
    // 生成左右边界的存在性证明
    merkle_proof_t *left_proof = merkle_tree_generate_proof(tree, left_boundary);
    merkle_proof_t *right_proof = merkle_tree_generate_proof(tree, right_boundary);
    
    if (!left_proof || !right_proof) {
        free(left_proof);
        free(right_proof);
        free(proof);
        return NULL;
    }
    
    proof->left_proof = *left_proof;
    proof->right_proof = *right_proof;
    
    free(left_proof);
    free(right_proof);
    
    return proof;
}

/**
 * 批量构建10万叶子节点的Merkle树
 */
merkle_tree_t* build_large_merkle_tree(size_t leaf_count) {
    printf("Building Merkle tree with %zu leaves...\n", leaf_count);
    
    merkle_tree_t *tree = merkle_tree_create(leaf_count);
    if (!tree) {
        printf("Failed to create Merkle tree\n");
        return NULL;
    }
    
    // 生成测试数据并添加叶子节点
    for (size_t i = 0; i < leaf_count; i++) {
        char data[64];
        snprintf(data, sizeof(data), "leaf_data_%zu", i);
        
        if (!merkle_tree_add_leaf(tree, (uint8_t*)data, strlen(data))) {
            printf("Failed to add leaf %zu\n", i);
            return NULL;
        }
        
        if (i % 10000 == 0) {
            printf("Added %zu leaves...\n", i);
        }
    }
    
    printf("Finalizing tree structure...\n");
    if (!merkle_tree_finalize(tree)) {
        printf("Failed to finalize tree\n");
        return NULL;
    }
    
    printf("Merkle tree built successfully!\n");
    printf("Tree height: %d\n", tree->height);
    printf("Root hash: ");
    for (int i = 0; i < SM3_DIGEST_SIZE; i++) {
        printf("%02x", tree->root->hash[i]);
    }
    printf("\n\n");
    
    return tree;
}

/**
 * 演示存在性和不存在性证明
 */
void demonstrate_merkle_proofs(merkle_tree_t *tree) {
    printf("=== Merkle Tree Proof Demonstration ===\n\n");
    
    // 测试存在性证明
    size_t test_indices[] = {0, 1000, 50000, 99999};
    size_t num_tests = sizeof(test_indices) / sizeof(test_indices[0]);
    
    for (size_t i = 0; i < num_tests; i++) {
        size_t index = test_indices[i];
        
        printf("Testing existence proof for leaf %zu:\n", index);
        
        // 生成证明
        merkle_proof_t *proof = merkle_tree_generate_proof(tree, index);
        if (!proof) {
            printf("  Failed to generate proof\n");
            continue;
        }
        
        // 重构叶子数据
        char leaf_data[64];
        snprintf(leaf_data, sizeof(leaf_data), "leaf_data_%zu", index);
        
        // 验证证明
        int valid = merkle_proof_verify(proof, (uint8_t*)leaf_data, strlen(leaf_data), tree->root->hash);
        printf("  Proof validation: %s\n", valid ? "PASSED" : "FAILED");
        printf("  Proof path length: %zu\n", proof->path_length);
        
        free(proof->path_hashes);
        free(proof->directions);
        free(proof);
        printf("\n");
    }
    
    // 测试不存在性证明
    size_t nonexistent_indices[] = {100000, 150000, 200000};
    size_t num_nonexistent = sizeof(nonexistent_indices) / sizeof(nonexistent_indices[0]);
    
    for (size_t i = 0; i < num_nonexistent; i++) {
        size_t index = nonexistent_indices[i];
        
        printf("Testing non-existence proof for index %zu:\n", index);
        
        merkle_nonexistence_proof_t *proof = merkle_tree_generate_nonexistence_proof(tree, index);
        if (!proof) {
            printf("  Failed to generate non-existence proof\n");
            continue;
        }
        
        printf("  Non-existence proof generated successfully\n");
        printf("  Left boundary: %zu, Right boundary: %zu\n", 
               proof->left_proof.leaf_index, proof->right_proof.leaf_index);
        
        free(proof);
        printf("\n");
    }
}
