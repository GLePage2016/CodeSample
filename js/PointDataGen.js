'use strict';

var moment = require('moment');
var PointDateUtils = require('./PointDateUtils');
var log = require('log4js').getLogger('gimbal');

class PointDataGen
{
    createDataDocKey(sId, pId, dKey)
    {
        var sIdStr = sId.toString();
        var pIdStr = pId.toString();

        return(sIdStr + ":" + pIdStr + ":" + dKey);
    }

    rawPointDataHourArray()
    {
        var rt = [];
        for (var i = 0; i < 60; i++)
        {
            rt.push(null);
        }
        return rt;
    }

    rawPointDataObject()
    {
        //Create 23 entry object the point hour arrays
        return (
            {
                '0':  this.rawPointDataHourArray(),
                '1':  this.rawPointDataHourArray(),
                '2':  this.rawPointDataHourArray(),
                '3':  this.rawPointDataHourArray(),
                '4':  this.rawPointDataHourArray(),
                '5':  this.rawPointDataHourArray(),
                '6':  this.rawPointDataHourArray(),
                '7':  this.rawPointDataHourArray(),
                '8':  this.rawPointDataHourArray(),
                '9':  this.rawPointDataHourArray(),
                '10': this.rawPointDataHourArray(),
                '11': this.rawPointDataHourArray(),
                '12': this.rawPointDataHourArray(),
                '13': this.rawPointDataHourArray(),
                '14': this.rawPointDataHourArray(),
                '15': this.rawPointDataHourArray(),
                '16': this.rawPointDataHourArray(),
                '17': this.rawPointDataHourArray(),
                '18': this.rawPointDataHourArray(),
                '19': this.rawPointDataHourArray(),
                '20': this.rawPointDataHourArray(),
                '21': this.rawPointDataHourArray(),
                '22': this.rawPointDataHourArray(),
                '23': this.rawPointDataHourArray()
            }
        );
    }
    
    rawPointDataDoc(sId, pId, dKey)
    {
        return (
        {
            _id: this.createDataDocKey(sId, pId, dKey),
            ptId: pId,
            ts: this.pointDateUtils.generateDateFromPointKey(dKey),
            data: this.rawPointDataObject()
        });
    }

    constructor(ptId, sId)
    {
        this.pointId = ptId;
        this.siteId = sId;

        //Point docs is an array of point data doc container objects
        //Each container object has two properties: dateKey: YYYYMMDD formatted and pointDoc: document itself
        this.pointDocKeys = [];
        this.pointDocs = [];
        this.pointDateUtils = new PointDateUtils();
    }

    addPointData(pointDttmStr, pointVal)
    {
        if (!pointDttmStr || pointDttmStr.length < 0)
        {
            return;
        }

        //var pointDttm = moment(pointDttmStr, moment.ISO_8601);
        //var dateKey = pointDttm.format("YYYYMMDD");
        var pointDttm = moment(pointDttmStr, 'YYYY-MM-DDTHH:mm:ss.SSS', true);
        var dateKey = this.pointDateUtils.generatePointKeyFromDate(pointDttm);

        //See if point docs contains a doc container with this date key
        var docKeyIdx = this.pointDocKeys.indexOf(dateKey);
        if (docKeyIdx == -1)
        {
            this.pointDocKeys.push(dateKey);
            this.pointDocs.push(
                {
                    dtKey: dateKey,
                    ptDoc: this.rawPointDataDoc(this.siteId, this.pointId, dateKey)
                });
            docKeyIdx = this.pointDocKeys.length-1;
        }

        var pointDoc = this.pointDocs[docKeyIdx].ptDoc;
        if (pointDoc)
        {
            //Lookup by the minute
            //Get hour string and minute value
            var hourKey = pointDttm.format('H');
            var minuteIdx = pointDttm.toObject().minutes;

            //set the point value and quality at the minute index
            var hourArray = pointDoc.data[hourKey];
            hourArray[minuteIdx] = pointVal;
        }
    }


    /**********************************************************************
     {
         "basPointData": [
         {
             "basId": "Roof Building OA Temperature",
             "timeseriesData": [
             {
                 "timestamp": "2015-02-23T10:15:00.000",
                 "value": "74.9"
                 },
                 {
                 "timestamp": "2015-02-23T10:30:00.000",
                 "value": "74.6"
                 },
                 {
                 "timestamp": "2015-02-23T10:45:00.000",
                 "value": "74.3"
             }
             ]
         }
         ]
     }
     */
    /**
     * Partially processed data coming from
     * basPointDataController.upsert functionality
     * By the time it gets here, we are just looking at the array
     *  pointed at by the 'basPointData' key in the request body
     * Reference array structure:
     * [
     *      'basId': 'Point A'
     *      'timeseriesData': [{
     *          'timestamp': 'YYYY-MM-DDTHH:MM:SS.fff'
     *          'value': '99.9'
     *      }]
     * ]
     * 
     * @param basPointDataArray
     * @param basKey
     * @returns {boolean}
     */
    processBasPointData(basPointDataArray, basKey)
    {
        var addedData = false;
        if (basPointDataArray && basPointDataArray.length > 0)
        {
            for (var i = 0; i < basPointDataArray.length; i++)
            {
                var curBasKeyElem = basPointDataArray[i];
                if (curBasKeyElem['basId'] == basKey)
                {
                    //Loop through all of the timestamp tuples for this matching basKey
                    var curBasKeyTimeData = curBasKeyElem['timeseriesData'];
                    if (curBasKeyTimeData && curBasKeyTimeData.length > 0)
                    {
                        for (var j = 0; j < curBasKeyTimeData.length; j++)
                        {
                            var curTimeTuple = curBasKeyTimeData[j];
                            this.addPointData(curTimeTuple['timestamp'], curTimeTuple['value'], true);
                            addedData = true;
                        }
                    }
                }
            }
        }

        return(addedData);
    }

    /*****************************************************************************
     "Bulk"
     /v1/sites/{siteId}/config/{versionId}/point-data
     - PUT/POST
     {
       "pointData": [
         { "id": 615196, "value": 13866.28565, "quality": 99, "timestamp": "2016-05-01T00:00:00.000" }
       ]
     }
     */

    //From the /sites/:siteId/config/:configId/point-data endpoint
    //We already know the point id from the constructor, so filter off of that
    processPointDataBulk(ptDataArray, pointKey)
    {
        if (!ptDataArray || ptDataArray.length <= 0 || !pointKey || pointKey.length <= 0)
        {
            return false;
        }

        if (ptDataArray && ptDataArray.length > 0)
        {
            for (var i = 0; i < ptDataArray.length; i++)
            {
                var curPt = ptDataArray[i];
                if (curPt && curPt['id'] == pointKey)
                {
                    this.addPointData(curPt['timestamp'], curPt['value'], false);
                }
            }
        }
        return true;
    }


    //From the /sites/:siteId/config/:configId/point-data endpoint
    //We already know the point id from the constructor, so filter off of that
    processPointDataBulkJSON(rawJSON, pointKey)
    {
        if (!rawJSON || rawJSON.length <= 0 || !pointKey || pointKey.length <= 0)
        {
            return false;
        }

        var pointDataArray = JSON.parse(rawJSON).pointData;
        return this.processPointDataBulk(pointDataArray, pointKey);
    }

    /*****************************************************************************
     "Single"
     {
        /v1/sites/24/config/{versionId}/points/{pointId}/point-data
        - GET
        [
          {
            "timestamp":"2016-02-01T00:10:00.000","value":0.0,"quality":99,"pointId":"616355"}
          }
        ]
     }
     */

    //From the /sites/:siteId/config/:configId/points/:pointid/point-data endpoint
    //This single envelope only comes from a request specifying the point id
    processPointDataJSON(rawJSON)
    {
        if (!rawJSON || rawJSON.length <= 0)
        {
            return false;
        }

        var pointData = JSON.parse(rawJSON);
        if (pointData)
        {
            var ptDataArray = pointData['pointData'];
            if (ptDataArray && ptDataArray.length > 0)
            {
                for (var i = 0; i < ptDataArray.length; i++)
                {
                    var curPt = ptDataArray[i];
                    if (curPt)
                    {
                        this.addPointData(curPt['timestamp'], curPt['value'], false);
                    }
                }
            }
        }
    }
}

module.exports = PointDataGen;
