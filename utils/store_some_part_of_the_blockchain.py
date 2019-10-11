#!/usr/bin/python3

from blockcypher import get_block_overview, get_transaction_details
from time import sleep
import json
from datetime import date, datetime

def json_serial(obj):
    if isinstance(obj, (datetime, date)):
        return obj.isoformat()
    raise TypeError ("Type %s not serializable" % type(obj))

blockHash = '0000000000000000001c9bb1ea94a9d7fd726069652d7454531b447e58310505'
numberOfRequests = 0
pendingTxsToFetch = []
nTxs = 0
processedTxs = 0

while True:
    numberOfRequests = numberOfRequests + 1
    if pendingTxsToFetch:
        try:
            txid = pendingTxsToFetch.pop()
            tx = get_transaction_details(txid)
            file = open("blockchain/txs/%s" % txid, "w")
            json.dump(tx, file, default = json_serial)
            file.close()
            processedTxs = processedTxs + 1
        except:
            pendingTxsToFetch.append(txid)
    else:
        if processedTxs == nTxs:
            # I have already fetched everything from the current block
            processedTxs = 0
        start = processedTxs
        limit = 500
        try:
            block = get_block_overview(blockHash, txn_offset = start, txn_limit = limit)
            file = open("blockchain/blocks/%s" % blockHash, "w")
            json.dump(block, file, default = json_serial)
            file.close()
            nTxs = block['n_tx']
            pendingTxsToFetch = block['txids']
            if (start + limit) >= nTxs:
                # If I can fetch the reminder of txs in the current block, then the next block will the parent of this one
                blockHash = block['prev_block']
        except:
            pass
    if numberOfRequests == 200:
        sleep(60 * 60 + 10)
        numberOfRequests = 0
    elif numberOfRequests % 3 == 0:
        sleep(10)

