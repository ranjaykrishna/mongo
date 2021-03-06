/**
 *    Copyright (C) 2013 10gen Inc.
 *
 *    This program is free software: you can redistribute it and/or  modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Affero General Public License for more details.
 *
 *    You should have received a copy of the GNU Affero General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *    As a special exception, the copyright holders give permission to link the
 *    code of portions of this program with the OpenSSL library under certain
 *    conditions as described in each individual source file and distribute
 *    linked combinations including the program with the OpenSSL library. You
 *    must comply with the GNU Affero General Public License in all respects for
 *    all of the code used other than as permitted herein. If you modify file(s)
 *    with this exception, you may extend this exception to your version of the
 *    file(s), but you are not obligated to do so. If you do not wish to do so,
 *    delete this exception statement from your version. If you delete this
 *    exception statement from all source files in the program, then also delete
 *    it in the license file.
 */

#pragma once

#include <string>

#include "mongo/base/disallow_copying.h"
#include "mongo/db/client.h"
#include "mongo/s/write_ops/batched_command_request.h"
#include "mongo/s/write_ops/batched_command_response.h"
#include "mongo/s/write_ops/batched_delete_document.h"
#include "mongo/s/write_ops/batched_update_document.h"
#include "mongo/util/mongoutils/str.h"

namespace mongo {

    class BSONObjBuilder;
    class CurOp;
    class OpCounters;
    class OpDebug;
    struct LastError;

    /**
     * An instance of WriteBatchExecutor is an object capable of issuing a write batch.
     */
    class WriteBatchExecutor {
        MONGO_DISALLOW_COPYING(WriteBatchExecutor);
    public:
        WriteBatchExecutor( const BSONObj& defaultWriteConcern,
                            Client* client,
                            OpCounters* opCounters,
                            LastError* le );

        /**
         * Issues writes with requested write concern.  Fills response with errors if problems
         * occur.
         */
        void executeBatch( const BatchedCommandRequest& request, BatchedCommandResponse* response );

    private:

        // TODO: This will change in the near future, but keep like this for now
        struct WriteStats {

            WriteStats() :
                    numInserted( 0 ), numModified( 0 ), numUpdated( 0 ),
                    numUpserted( 0 ), numDeleted( 0 ) {
            }

            int numInserted;

            // The number of docs modified and updated (inc no-ops).
            // NOTE: The difference of the two are the number of no-ops.
            int numModified;
            int numUpdated;

            int numUpserted;
            int numDeleted;
        };

        /**
         * Issues the single write 'itemRef'. Returns true iff write item was issued
         * sucessfully and increments 'stats'. If the item is an upsert, fills in the
         * 'upsertedID' also, with the '_id' chosen for that update. If the write failed,
         * returns false and populates 'error'
         */
        bool applyWriteItem( const BatchItemRef& itemRef,
                             WriteStats* stats,
                             BSONObj* upsertedID,
                             WriteErrorDetail* error );

        //
        // Helpers to issue underlying write.
        // Returns true iff write item was issued sucessfully and increments stats, populates error
        // if not successful.
        //

        bool doWrite( const string& ns,
                      Client::Context& ctx,
                      const BatchItemRef& itemRef,
                      CurOp* currentOp,
                      WriteStats* stats,
                      BSONObj* upsertedID,
                      WriteErrorDetail* error );

        bool doInsert( const std::string& ns,
                       Client::Context& ctx,
                       const BSONObj& insertOp,
                       CurOp* currentOp,
                       WriteStats* stats,
                       WriteErrorDetail* error );

        bool doUpdate( const std::string& ns,
                       Client::Context& ctx,
                       const BatchedUpdateDocument& updateOp,
                       CurOp* currentOp,
                       WriteStats* stats,
                       BSONObj* upsertedID,
                       WriteErrorDetail* error );

        bool doDelete( const std::string& ns,
                       Client::Context& ctx,
                       const BatchedDeleteDocument& deleteOp,
                       CurOp* currentOp,
                       WriteStats* stats,
                       WriteErrorDetail* error );

        // Default write concern, if one isn't provide in the batches.
        const BSONObj _defaultWriteConcern;

        // Client object to issue writes on behalf of.
        // Not owned here.
        Client* _client;

        // OpCounters object to update.
        // Not owned here.
        OpCounters* _opCounters;

        // LastError object to use for preparing write results.
        // Not owned here.
        LastError* _le;

    };

} // namespace mongo
