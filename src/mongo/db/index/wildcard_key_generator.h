/**
 * Copyright (C) 2018 MongoDB Inc.
 *
 * This program is free software: you can redistribute it and/or  modify
 * it under the terms of the GNU Affero General Public License, version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, the copyright holders give permission to link the
 * code of portions of this program with the OpenSSL library under certain
 * conditions as described in each individual source file and distribute
 * linked combinations including the program with the OpenSSL library. You
 * must comply with the GNU Affero General Public License in all respects
 * for all of the code used other than as permitted herein. If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so. If you do not
 * wish to do so, delete this exception statement from your version. If you
 * delete this exception statement from all source files in the program,
 * then also delete it in the license file.
 */

#pragma once

#include "mongo/db/exec/projection_exec_agg.h"
#include "mongo/db/field_ref.h"
#include "mongo/db/query/collation/collator_interface.h"

namespace mongo {

/**
 * This class is responsible for generating an aggregation projection based on the keyPattern and
 * pathProjection specs, and for subsequently extracting the set of all path-value pairs for each
 * document.
 */
class WildcardKeyGenerator {
public:
    static constexpr StringData kSubtreeSuffix = ".$**"_sd;

    /**
     * Returns an owned ProjectionExecAgg identical to the one that WildcardKeyGenerator will use
     * internally when generating the keys for the $** index, as defined by the 'keyPattern' and
     * 'pathProjection' arguments.
     */
    static std::unique_ptr<ProjectionExecAgg> createProjectionExec(BSONObj keyPattern,
                                                                   BSONObj pathProjection);

    WildcardKeyGenerator(BSONObj keyPattern,
                         BSONObj pathProjection,
                         const CollatorInterface* collator);

    /**
     * Applies the appropriate Wildcard projection to the input doc, and then adds one key-value
     * pair to the BSONObjSet 'keys' for each leaf node in the post-projection document:
     *      { '': 'path.to.field', '': <collation-aware-field-value> }
     * Also adds one entry to 'multikeyPaths' for each array encountered in the post-projection
     * document, in the following format:
     *      { '': 1, '': 'path.to.array' }
     */
    void generateKeys(BSONObj inputDoc, BSONObjSet* keys, BSONObjSet* multikeyPaths) const;

private:
    // Traverses every path of the post-projection document, adding keys to the set as it goes.
    void _traverseWildcard(BSONObj obj,
                           bool objIsArray,
                           FieldRef* path,
                           BSONObjSet* keys,
                           BSONObjSet* multikeyPaths) const;

    // Helper functions to format the entry appropriately before adding it to the key/path tracker.
    void _addMultiKey(const FieldRef& fullPath, BSONObjSet* multikeyPaths) const;
    void _addKey(BSONElement elem, const FieldRef& fullPath, BSONObjSet* keys) const;

    // Helper to check whether the element is a nested array, and conditionally add it to 'keys'.
    bool _addKeyForNestedArray(BSONElement elem,
                               const FieldRef& fullPath,
                               bool enclosingObjIsArray,
                               BSONObjSet* keys) const;
    bool _addKeyForEmptyLeaf(BSONElement elem, const FieldRef& fullPath, BSONObjSet* keys) const;

    std::unique_ptr<ProjectionExecAgg> _projExec;
    const CollatorInterface* _collator;
    const BSONObj _keyPattern;
};
}  // namespace mongo
