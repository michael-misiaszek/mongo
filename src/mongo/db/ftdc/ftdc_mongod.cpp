/**
 * Copyright (C) 2015 MongoDB Inc.
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

#include "mongo/platform/basic.h"

#include "mongo/db/ftdc/ftdc_mongod.h"

#include <boost/filesystem.hpp>

#include "mongo/db/ftdc/constants.h"
#include "mongo/db/ftdc/controller.h"
#include "mongo/db/ftdc/ftdc_server.h"
#include "mongo/db/repl/replication_coordinator.h"
#include "mongo/db/repl/replication_coordinator_global.h"
#include "mongo/db/storage/storage_options.h"

namespace mongo {

namespace {
void registerMongoDCollectors(FTDCController* controller) {
    // These metrics are only collected if replication is enabled
    if (repl::getGlobalReplicationCoordinator()->getReplicationMode() !=
        repl::ReplicationCoordinator::modeNone) {
        // CmdReplSetGetStatus
        controller->addPeriodicCollector(stdx::make_unique<FTDCSimpleInternalCommandCollector>(
            "replSetGetStatus", "replSetGetStatus", "", BSON("replSetGetStatus" << 1)));

        // CollectionStats
        controller->addPeriodicCollector(
            stdx::make_unique<FTDCSimpleInternalCommandCollector>("collStats",
                                                                  "local.oplog.rs.stats",
                                                                  "local",
                                                                  BSON("collStats"
                                                                       << "oplog.rs")));
    }
}

}  // namespace

void startMongoDFTDC() {
    boost::filesystem::path dir(storageGlobalParams.dbpath);
    dir /= kFTDCDefaultDirectory.toString();

    startFTDC(dir, FTDCStartMode::kStart, registerMongoDCollectors);
}

void stopMongoDFTDC() {
    stopFTDC();
}

}  // namespace mongo