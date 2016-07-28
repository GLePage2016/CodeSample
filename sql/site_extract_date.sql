use Asset_Diag 
go

------------------------------------------------------------------------------
-- Single Asset Tree Site Extraction Script
--
------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Create the DropProcedureWithCheck method in case it does not exist
-------------------------------------------------------------------------------
IF EXISTS ( SELECT  *
            FROM    sys.objects
            WHERE   object_id = OBJECT_ID(N'DropProcedureWithCheck')
                    AND type IN ( N'P', N'PC' ) ) 
BEGIN
	DROP PROCEDURE [dbo].DropProcedureWithCheck
END
GO

CREATE PROCEDURE [dbo].DropProcedureWithCheck @ProcName nvarchar(1000)
AS
BEGIN
	DECLARE @DropProcSQL varchar(1000)
	DECLARE @LogMsg varchar(355)

	IF EXISTS ( SELECT  *
				FROM    sys.objects
				WHERE   object_id = OBJECT_ID(@ProcName)
						AND type IN ( N'P', N'PC' ) ) 
	BEGIN
		BEGIN TRANSACTION DropProcTrans			
		SELECT @DropProcSQL = 'USE ' + DB_NAME() + CHAR(10)
		SET @DropProcSQL = @DropProcSQL + 'DROP PROCEDURE ' + @ProcName
		EXEC(@DropProcSQL)
		
		IF @@ERROR<>0
		BEGIN
			SET @LogMsg = 'Could not drop procedure ' + @ProcName
			RAISERROR(@LogMsg, 0, 1) WITH NOWAIT			
			ROLLBACK TRANSACTION DropProcTrans
		END
		ELSE
		BEGIN
			SET @LogMsg = 'Dropped procedure ' + @ProcName
			RAISERROR(@LogMsg, 0, 1) WITH NOWAIT
			COMMIT TRANSACTION DropProcTrans
		END
	END
	ELSE
	BEGIN
		SET @LogMsg = 'Procedure ' + @ProcName + ' does not exist'
		RAISERROR(@LogMsg, 0, 1) WITH NOWAIT
	END
END
GO

-------------------------------------------------------------------------------
-- Client
-------------------------------------------------------------------------------
EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractClient'
GO

CREATE PROCEDURE [dbo].ExtractClient(@SiteName nvarchar(200))
AS
BEGIN
	SET NOCOUNT ON
	SELECT CL.*
	FROM [Asset_Diag].[dbo].[Client] CL
	WHERE CL.ClientID IN (select ClientID from Asset_Diag.dbo.Site where Name = @SiteName)
END
GO

-- All Clients
EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractClientAll'
GO

CREATE PROCEDURE [dbo].ExtractClientAll
AS
BEGIN
	SET NOCOUNT ON
	SELECT CL.*
	FROM [Asset_Diag].[dbo].[Client] CL	
END
GO

	
-------------------------------------------------------------------------------
-- Site
-------------------------------------------------------------------------------
EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractSite'
GO

CREATE PROCEDURE [dbo].ExtractSite(@SiteName nvarchar(200))
AS
BEGIN
	SET NOCOUNT ON
	SELECT *
	FROM [Asset_Diag].[dbo].[Site] ST WHERE ST.Name = @SiteName
END
GO

-- All Sites
EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractSiteAll'
GO

CREATE PROCEDURE [dbo].ExtractSiteAll
AS
BEGIN
	SET NOCOUNT ON
	SELECT * FROM Asset_Diag.dbo.Site 
END
GO

-------------------------------------------------------------------------------
-- Asset
-------------------------------------------------------------------------------
EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractAsset'
GO

CREATE PROCEDURE [dbo].ExtractAsset(@SiteName nvarchar(200))
AS
BEGIN
	SET NOCOUNT ON
	select ass.* from Asset_Diag.dbo.Asset ass 
	inner join Asset_Diag.dbo.SiteAsset sass on sass.AssetID = ass.AssetID
	where sass.SiteID in (select SiteID from Asset_Diag.dbo.Site where Name = @SiteName);
END
GO

EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractAssetAll'
GO

CREATE PROCEDURE [dbo].ExtractAssetAll
AS
BEGIN
	SET NOCOUNT ON
	select * from Asset_Diag.dbo.Asset
END
GO

-------------------------------------------------------------------------------
-- Attribute Data
-------------------------------------------------------------------------------
EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractAttributeData'
GO

CREATE PROCEDURE [dbo].ExtractAttributeData(@SiteName nvarchar(200))
AS
BEGIN
	SET NOCOUNT ON
	select attr.* from Asset_Diag.dbo.AttributeData attr
	inner join Asset_Diag.dbo.Asset ass on ass.AssetID = attr.AssetID
	inner join Asset_Diag.dbo.SiteAsset sass on sass.AssetID = ass.AssetID
	where sass.SiteID in (select SiteID from Asset_Diag.dbo.Site where Name = @SiteName);
END
GO

EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractAttributeDataAll'
GO

CREATE PROCEDURE [dbo].ExtractAttributeDataAll
AS
BEGIN
	SET NOCOUNT ON
	select * from Asset_Diag.dbo.AttributeData
END
GO

-------------------------------------------------------------------------------
-- SiteAsset
-------------------------------------------------------------------------------
EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractSiteAsset'
GO

CREATE PROCEDURE [dbo].ExtractSiteAsset(@SiteName nvarchar(200))
AS
BEGIN
	SET NOCOUNT ON
	select sass.* from Asset_Diag.dbo.SiteAsset sass
	where sass.SiteID in (select SiteID from Asset_Diag.dbo.Site where Name = @SiteName);
END
GO

EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractSiteAssetAll'
GO

CREATE PROCEDURE [dbo].ExtractSiteAssetAll
AS
BEGIN
	SET NOCOUNT ON
	select * from Asset_Diag.dbo.SiteAsset
END
GO

-------------------------------------------------------------------------------
-- AddressSimple
-------------------------------------------------------------------------------
EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractAddressSimple'
GO

CREATE PROCEDURE [dbo].ExtractAddressSimple(@SiteName nvarchar(200))
AS
BEGIN
	SET NOCOUNT ON
	select addr.* from Asset_Diag.dbo.AddressSimple addr
	inner join Asset_Diag.dbo.SiteAddressSimple saddr on addr.AddressSimpleID = saddr.AddressSimpleID
	inner join Asset_Diag.dbo.Site sit on saddr.SiteID = sit.SiteID
	where sit.Name = @SiteName;
END
GO

EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractAddressSimpleAll'
GO

CREATE PROCEDURE [dbo].ExtractAddressSimpleAll
AS
BEGIN
	SET NOCOUNT ON
	select * from Asset_Diag.dbo.AddressSimple
END
GO

-------------------------------------------------------------------------------
-- SiteAddressSimple
-------------------------------------------------------------------------------
EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractSiteAddressSimple'
GO

CREATE PROCEDURE [dbo].ExtractSiteAddressSimple(@SiteName nvarchar(200))
AS
BEGIN
	SET NOCOUNT ON
	select saddr.* from Asset_Diag.dbo.SiteAddressSimple saddr 
	inner join Asset_Diag.dbo.Site sit on saddr.SiteID = sit.SiteID
	where sit.Name = @SiteName;
END
GO

EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractSiteAddressSimpleAll'
GO

CREATE PROCEDURE [dbo].ExtractSiteAddressSimpleAll
AS
BEGIN
	SET NOCOUNT ON
	select * from Asset_Diag.dbo.SiteAddressSimple
END
GO

-------------------------------------------------------------------------------
-- Point
-- Must extract via PointBase, BasPoint, PointMap, and PointMapDetails
-- PointBase is by AssetID
-------------------------------------------------------------------------------
EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractPointBase'
GO

CREATE PROCEDURE [dbo].ExtractPointBase(@SiteName nvarchar(200))
AS
BEGIN
	SET NOCOUNT ON
	select pb.* from Asset_Diag.dbo.PointBase pb
	inner join Asset_Diag.dbo.Asset ass on pb.AssetID = ass.AssetID
	inner join Asset_Diag.dbo.SiteAsset sass on sass.AssetID = ass.AssetID
	where sass.SiteID in (select SiteID from Asset_Diag.dbo.Site where Name = @SiteName);
END
GO
EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractPointBaseAll'
GO

CREATE PROCEDURE [dbo].ExtractPointBaseAll
AS
BEGIN
	SET NOCOUNT ON
	select * from Asset_Diag.dbo.PointBase
END
GO

-------------------------------------------------------------------------------
-- BasPoint is by PointMapID to the Point's pulled for PointBase by AssetID
-------------------------------------------------------------------------------
EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractBasPoint'
GO

CREATE PROCEDURE [dbo].ExtractBasPoint(@SiteName nvarchar(200))
AS
BEGIN
	SET NOCOUNT ON
	select distinct bap.* from Asset_Diag.dbo.BasPoint bap 
	inner join Asset_Diag.dbo.PointBase pb on pb.PointMapID = bap.PointMapID
	inner join Asset_Diag.dbo.Asset ass on pb.AssetID = ass.AssetID
	inner join Asset_Diag.dbo.SiteAsset sass on sass.AssetID = ass.AssetID
	where sass.SiteID in (select SiteID from Asset_Diag.dbo.Site where Name = @SiteName);
END
GO

EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractBasPointAll'
GO

CREATE PROCEDURE [dbo].ExtractBasPointAll
AS
BEGIN
	SET NOCOUNT ON
	select * from Asset_Diag.dbo.BasPoint
END
GO

-------------------------------------------------------------------------------
-- PointMap is by PointMapID to the Point's pulled for PointBase by AssetID
-------------------------------------------------------------------------------
EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractPointMap'
GO

CREATE PROCEDURE [dbo].ExtractPointMap(@SiteName nvarchar(200))
AS
BEGIN
	SET NOCOUNT ON
	select distinct pmap.* from Asset_Diag.dbo.PointMap pmap
	inner join Asset_Diag.dbo.PointBase pb on pb.PointMapID = pmap.PointMapID
	inner join Asset_Diag.dbo.Asset ass on pb.AssetID = ass.AssetID
	inner join Asset_Diag.dbo.SiteAsset sass on sass.AssetID = ass.AssetID
	where sass.SiteID in (select SiteID from Asset_Diag.dbo.Site where Name = @SiteName);
END
GO

EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractPointMapAll'
GO

CREATE PROCEDURE [dbo].ExtractPointMapAll
AS
BEGIN
	SET NOCOUNT ON
	select * from Asset_Diag.dbo.PointMap
END
GO

-------------------------------------------------------------------------------
-- PointMapDetails
-------------------------------------------------------------------------------
EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractPointMapDetails'
GO

CREATE PROCEDURE [dbo].ExtractPointMapDetails(@SiteName nvarchar(200))
AS
BEGIN
	SET NOCOUNT ON
	select distinct pmapdet.* from Asset_Diag.dbo.PointMapDetails pmapdet 
	inner join Asset_Diag.dbo.PointBase pb on pb.PointMapID = pmapdet.PointMapID
	inner join Asset_Diag.dbo.Asset ass on pb.AssetID = ass.AssetID
	inner join Asset_Diag.dbo.SiteAsset sass on sass.AssetID = ass.AssetID
	where sass.SiteID in (select SiteID from Asset_Diag.dbo.Site where Name = @SiteName);
END
GO

EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractPointMapDetailsAll'
GO

CREATE PROCEDURE [dbo].ExtractPointMapDetailsAll
AS
BEGIN
	SET NOCOUNT ON
	select * from Asset_Diag.dbo.PointMapDetails
END
GO

-------------------------------------------------------------------------------
-- Users
-------------------------------------------------------------------------------
EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractUsers'
GO

CREATE PROCEDURE [dbo].ExtractUsers
AS
BEGIN
	SET NOCOUNT ON
	select id, first_name, last_name, email, encrypted_password,
	reset_password_token, reset_password_sent_at, remember_created_at,
	sign_in_count, 
	current_sign_in_at, last_sign_in_at,
	current_sign_in_ip, last_sign_in_ip,
	created_at, updated_at,
	NULL as restricted_to_site_id,
	can_view_all_clients,
	api_key,
	preferred_units,
	most_recent_failed_sign_in_at,
	consecutive_failed_sign_in_count,
	most_recent_sign_out_at,
	show_data_integrity,
	show_energy_cost,
	require_password_change,
	account_permanently_locked,
	invitation_token,
	invitation_sent_at,
	password_lock_token,
	password_lock_sent_at,
	show_function_overrides,
	show_meter_consumption_overrides from Asset_Diag.pf.Users 
END
GO

-------------------------------------------------------------------------------
-- Role
-------------------------------------------------------------------------------
EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractRole'
GO

CREATE PROCEDURE [dbo].ExtractRole
AS
BEGIN
	SET NOCOUNT ON
	select rol.* from Asset_Diag.pf.Role rol
END
GO

-------------------------------------------------------------------------------
-- UserClient
-------------------------------------------------------------------------------
EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractUserClient'
GO

CREATE PROCEDURE [dbo].ExtractUserClient
AS
BEGIN
	SET NOCOUNT ON
	select usrc.* from Asset_Diag.pf.UserClient usrc
END
GO

-------------------------------------------------------------------------------
-- UserRole
-------------------------------------------------------------------------------
EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractUserRole'
GO

CREATE PROCEDURE [dbo].ExtractUserRole
AS
BEGIN
	SET NOCOUNT ON
	select usrrol.* from Asset_Diag.pf.UserRole usrrol
END
GO

-------------------------------------------------------------------------------
-- Schedule
-------------------------------------------------------------------------------
EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractSchedule'
GO

CREATE PROCEDURE [dbo].ExtractSchedule(@SiteName nvarchar(200))
AS
BEGIN
	SET NOCOUNT ON
	select sched.* from Asset_Diag.dbo.Schedule sched
	inner join Asset_Diag.dbo.Site sit on sched.SiteID = sit.SiteID
	where Sit.Name = @SiteName;
END
GO

EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractScheduleAll'
GO

CREATE PROCEDURE [dbo].ExtractScheduleAll
AS
BEGIN
	SET NOCOUNT ON
	select sched.* from Asset_Diag.dbo.Schedule sched
END
GO

-------------------------------------------------------------------------------
-- ScheduleChangeLog
-------------------------------------------------------------------------------
EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractScheduleChangeLog'
GO

CREATE PROCEDURE [dbo].ExtractScheduleChangeLog(@SiteName nvarchar(200))
AS
BEGIN
	SET NOCOUNT ON
	
	select schedcl.* from Asset_Diag.dbo.ScheduleChangeLog schedcl
	inner join Asset_Diag.dbo.Schedule sched on schedcl.ScheduleID = sched.ScheduleID
	inner join Asset_Diag.dbo.Site sit on sched.SiteID = sit.SiteID
	where Sit.Name = @SiteName;
END
GO

EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractScheduleChangeLogAll'
GO

CREATE PROCEDURE [dbo].ExtractScheduleChangeLogAll
AS
BEGIN
	SET NOCOUNT ON
	select schedcl.* from Asset_Diag.dbo.ScheduleChangeLog schedcl
END
GO

-------------------------------------------------------------------------------
-- ScheduleDetail
-------------------------------------------------------------------------------
EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractScheduleDetail'
GO

CREATE PROCEDURE [dbo].ExtractScheduleDetail(@SiteName nvarchar(200))
AS
BEGIN
	SET NOCOUNT ON
	select scheddet.* from Asset_Diag.dbo.ScheduleDetail scheddet	
	inner join Asset_Diag.dbo.Schedule sched on scheddet.ScheduleID = sched.ScheduleID
	inner join Asset_Diag.dbo.Site sit on sched.SiteID = sit.SiteID
	where Sit.Name = @SiteName;
END
GO

EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractScheduleDetailAll'
GO

CREATE PROCEDURE [dbo].ExtractScheduleDetailAll
AS
BEGIN
	SET NOCOUNT ON
	select sched.* from Asset_Diag.dbo.ScheduleDetail sched
END
GO

-------------------------------------------------------------------------------
-- ScheduleDetailChangeLog
-------------------------------------------------------------------------------
EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractScheduleDetailChangeLog'
GO

CREATE PROCEDURE [dbo].ExtractScheduleDetailChangeLog(@SiteName nvarchar(200))
AS
BEGIN
	SET NOCOUNT ON
	select distinct scheddetlog.* from Asset_Diag.dbo.ScheduleDetailChangeLog scheddetlog
	inner join Asset_Diag.dbo.ScheduleDetail scheddet on scheddet.ScheduleDetailID = scheddetlog.ScheduleDetailID
	inner join Asset_Diag.dbo.Schedule sched on scheddet.ScheduleID = sched.ScheduleID
	inner join Asset_Diag.dbo.Site sit on sched.SiteID = sit.SiteID
	where Sit.Name = @SiteName;
END
GO

EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractScheduleDetailChangeLogAll'
GO

CREATE PROCEDURE [dbo].ExtractScheduleDetailChangeLogAll
AS
BEGIN
	SET NOCOUNT ON
	select scheddetlog.* from Asset_Diag.dbo.ScheduleDetailChangeLog scheddetlog
END
GO

-------------------------------------------------------------------------------
-- ScheduleException
-------------------------------------------------------------------------------
EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractScheduleException'
GO

CREATE PROCEDURE [dbo].ExtractScheduleException(@SiteName nvarchar(200))
AS
BEGIN
	SET NOCOUNT ON
	select schedex.* from Asset_Diag.dbo.ScheduleException schedex
	inner join Asset_Diag.dbo.Schedule sched on sched.ScheduleID = schedex.ScheduleID
	inner join Asset_Diag.dbo.Site sit on sched.SiteID = sit.SiteID
	where Sit.Name = @SiteName;
END
GO

EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractScheduleExceptionAll'
GO

CREATE PROCEDURE [dbo].ExtractScheduleExceptionAll
AS
BEGIN
	SET NOCOUNT ON
	select schedex.* from Asset_Diag.dbo.ScheduleException schedex
END
GO
-------------------------------------------------------------------------------
-- ScheduleExceptionChangeLog
-------------------------------------------------------------------------------
EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractScheduleExceptionChangeLog'
GO

CREATE PROCEDURE [dbo].ExtractScheduleExceptionChangeLog(@SiteName nvarchar(200))
AS
BEGIN
	SET NOCOUNT ON
	select schedexcl.* from Asset_Diag.dbo.ScheduleExceptionChangeLog schedexcl
	inner join Asset_Diag.dbo.ScheduleException schedex on schedexcl.ScheduleExceptionID = schedex.ScheduleExceptionID
	inner join Asset_Diag.dbo.Schedule sched on sched.ScheduleID = schedex.ScheduleID
	inner join Asset_Diag.dbo.Site sit on sched.SiteID = sit.SiteID
	where Sit.Name = @SiteName;
END
GO

EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractScheduleExceptionChangeLogAll'
GO

CREATE PROCEDURE [dbo].ExtractScheduleExceptionChangeLogAll
AS
BEGIN
	SET NOCOUNT ON
	select schedexcl.* from Asset_Diag.dbo.ScheduleExceptionChangeLog schedexcl
END
GO
-------------------------------------------------------------------------------
-- ScheduleModes
-------------------------------------------------------------------------------
EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractScheduleModes'
GO

CREATE PROCEDURE [dbo].ExtractScheduleModes
AS
BEGIN
	SET NOCOUNT ON
	select schedmode.* from Asset_Diag.dbo.ScheduleModes schedmode
END
GO
-------------------------------------------------------------------------------
-- ScheduleRecurrenceDays
-------------------------------------------------------------------------------
EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractScheduleRecurrenceDays'
GO

CREATE PROCEDURE [dbo].ExtractScheduleRecurrenceDays
AS
BEGIN
	SET NOCOUNT ON
	select schedrecdays.* from Asset_Diag.dbo.ScheduleRecurrenceDays schedrecdays
END
GO
-------------------------------------------------------------------------------
-- ScheduleRecurrenceFrequency
-------------------------------------------------------------------------------
EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractScheduleRecurrenceFrequency'
GO

CREATE PROCEDURE [dbo].ExtractScheduleRecurrenceFrequency
AS
BEGIN
	SET NOCOUNT ON
	select schedrecfreq.* from Asset_Diag.dbo.ScheduleRecurrenceFrequency schedrecfreq
END
GO
-------------------------------------------------------------------------------
-- ScheduleRecurrenceMonth
-------------------------------------------------------------------------------
EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractScheduleRecurrenceMonth'
GO

CREATE PROCEDURE [dbo].ExtractScheduleRecurrenceMonth
AS
BEGIN
	SET NOCOUNT ON
	select schedrecmon.* from Asset_Diag.dbo.ScheduleRecurrenceFrequency schedrecmon
END
GO
-------------------------------------------------------------------------------
-- ScheduleRecurType
-------------------------------------------------------------------------------
EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractScheduleRecurType'
GO

CREATE PROCEDURE [dbo].ExtractScheduleRecurType
AS
BEGIN
	SET NOCOUNT ON
	select schedrectype.* from Asset_Diag.dbo.ScheduleRecurType schedrectype
END
GO
-------------------------------------------------------------------------------
-- AssetSchedule
-------------------------------------------------------------------------------
EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractAssetSchedule'
GO

CREATE PROCEDURE [dbo].ExtractAssetSchedule(@SiteName nvarchar(200))
AS
BEGIN
	SET NOCOUNT ON
	select asched.* from Asset_Diag.dbo.AssetSchedule asched
	inner join Asset_Diag.dbo.SiteAsset sass on sass.AssetID = asched.AssetID
	inner join Asset_Diag.dbo.Site sit on sass.SiteID = sit.SiteID
	where sit.Name = @SiteName;
END
GO

EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractAssetScheduleAll'
GO

CREATE PROCEDURE [dbo].ExtractAssetScheduleAll
AS
BEGIN
	SET NOCOUNT ON
	select sched.* from Asset_Diag.dbo.Schedule sched
END
GO
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
-- Faults start (need start and end date)
-------------------------------------------------------------------------------
EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractFaults'
GO

CREATE PROCEDURE [dbo].ExtractFaults(@SiteName nvarchar(200), @StartDate datetime, @EndDate datetime)
AS
BEGIN
	SET NOCOUNT ON
	select ft.FaultID, ft.ClientID, ft.SiteID, ft.AssetID,
	       ft.Status,ft.FunctionID,ft.OriginalMessageID,ft.MessageID,
		   ft.StandardValue,ft.StandardTarget,ft.StandardUOM,ft.MetricValue,
		   ft.MetricTarget,ft.MetricUOM,ft.Text,ft.PriorityID,ft.TicketID,ft.Rank,
		   ft.OwnerID,ft.FaultTimeThreshold
	from Asset_Diag.dbo.Faults ft
	inner join Asset_Diag.dbo.Asset ass on ft.AssetID = ass.AssetID
	inner join Asset_Diag.dbo.SiteAsset sass on sass.AssetID = ass.AssetID
	where 
		(ft.StartDTTM between @StartDate AND @EndDate
		 and
		 ft.EndDTTM between @StartDate and @enddate)
	and sass.SiteID in (select SiteID from Asset_Diag.dbo.Site where Name = @SiteName)
END
GO

EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractFaultsAllSite'
GO

CREATE PROCEDURE [dbo].ExtractFaultsAllSite(@SiteName nvarchar(200))
AS
BEGIN
	SET NOCOUNT ON
	select ft.FaultID, ft.ClientID, ft.SiteID, ft.AssetID,
	       ft.Status,ft.FunctionID,ft.OriginalMessageID,ft.MessageID,
		   ft.StandardValue,ft.StandardTarget,ft.StandardUOM,ft.MetricValue,
		   ft.MetricTarget,ft.MetricUOM,ft.Text,ft.PriorityID,ft.TicketID,ft.Rank,
		   ft.OwnerID,ft.FaultTimeThreshold
	from Asset_Diag.dbo.Faults ft
	inner join Asset_Diag.dbo.Asset ass on ft.AssetID = ass.AssetID
	inner join Asset_Diag.dbo.SiteAsset sass on sass.AssetID = ass.AssetID
	where sass.SiteID in (select SiteID from Asset_Diag.dbo.Site where Name = @SiteName)
END
GO


EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractFaultsAllDate'
GO

CREATE PROCEDURE [dbo].ExtractFaultsAllDate(@StartDate datetime, @EndDate datetime)
AS
BEGIN
	SET NOCOUNT ON
	select ft.FaultID, ft.ClientID, ft.SiteID, ft.AssetID,
	       ft.Status,ft.FunctionID,ft.OriginalMessageID,ft.MessageID,
		   ft.StandardValue,ft.StandardTarget,ft.StandardUOM,ft.MetricValue,
		   ft.MetricTarget,ft.MetricUOM,ft.Text,ft.PriorityID,ft.TicketID,ft.Rank,
		   ft.OwnerID,ft.FaultTimeThreshold
	from Asset_Diag.dbo.Faults ft
	where 
		(ft.StartDTTM between @StartDate AND @EndDate
		 and
		 ft.EndDTTM between @StartDate and @enddate)	
END
GO

EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractFaultsAll'
GO

CREATE PROCEDURE [dbo].ExtractFaultsAll
AS
BEGIN
	SET NOCOUNT ON
	select ft.FaultID, ft.ClientID, ft.SiteID, ft.AssetID,
	       ft.Status,ft.FunctionID,ft.OriginalMessageID,ft.MessageID,
		   ft.StandardValue,ft.StandardTarget,ft.StandardUOM,ft.MetricValue,
		   ft.MetricTarget,ft.MetricUOM,ft.Text,ft.PriorityID,ft.TicketID,ft.Rank,
		   ft.OwnerID,ft.FaultTimeThreshold
	from Asset_Diag.dbo.Faults ft
END
GO

-------------------------------------------------------------------------------
--Fault Occurrence (need start and end date)
-------------------------------------------------------------------------------
EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractFaultOccurrence'
GO

CREATE PROCEDURE [dbo].ExtractFaultOccurrence(@SiteName nvarchar(200), @StartDate datetime, @EndDate datetime)
AS
BEGIN
	SET NOCOUNT ON
	select foc.FaultOccurrenceID, foc.FaultId from Asset_Diag.dbo.FaultOccurrence foc
	inner join Asset_Diag.dbo.Faults ft on foc.FaultId = ft.FaultID
	inner join Asset_Diag.dbo.Asset ass on ft.AssetID = ass.AssetID
	inner join Asset_Diag.dbo.SiteAsset sass on sass.AssetID = ass.AssetID
	where 
		(ft.StartDTTM between @StartDate and @EndDate AND 
		 ft.EndDTTM between @StartDate AND @EndDate) 
	and sass.SiteID in (select SiteID from Asset_Diag.dbo.Site where Name = @SiteName)
END
GO

EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractFaultOccurrenceAllSite'
GO

CREATE PROCEDURE [dbo].ExtractFaultOccurrenceAllSite(@SiteName nvarchar(200))
AS
BEGIN
	SET NOCOUNT ON
	select foc.FaultOccurrenceID, foc.FaultId from Asset_Diag.dbo.FaultOccurrence foc
	inner join Asset_Diag.dbo.Faults ft on foc.FaultId = ft.FaultID
	inner join Asset_Diag.dbo.Asset ass on ft.AssetID = ass.AssetID
	inner join Asset_Diag.dbo.SiteAsset sass on sass.AssetID = ass.AssetID
	where sass.SiteID in (select SiteID from Asset_Diag.dbo.Site where Name = @SiteName)
END
GO
	
EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractFaultOccurrenceAllDate'
GO

CREATE PROCEDURE [dbo].ExtractFaultOccurrenceAllDate(@StartDate datetime, @EndDate datetime)
AS
BEGIN
	SET NOCOUNT ON
	select foc.FaultOccurrenceID, foc.FaultId from Asset_Diag.dbo.FaultOccurrence foc
	inner join Asset_Diag.dbo.Faults ft on foc.FaultId = ft.FaultID
	where 
		(ft.StartDTTM between @StartDate and @EndDate AND 
		 ft.EndDTTM between @StartDate AND @EndDate) 	
END
GO

EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractFaultOccurrenceAll'
GO

CREATE PROCEDURE [dbo].ExtractFaultOccurrenceAll
AS
BEGIN
	SET NOCOUNT ON
	select foc.FaultOccurrenceID, foc.FaultId from Asset_Diag.dbo.FaultOccurrence foc
END
GO

-------------------------------------------------------------------------------
--FaultDayOccurrence (need start and end date)
-------------------------------------------------------------------------------
EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractFaultDayOccurrence'
GO

CREATE PROCEDURE [dbo].ExtractFaultDayOccurrence(@SiteName nvarchar(200), @StartDate datetime, @EndDate datetime)
AS
BEGIN
	SET NOCOUNT ON
	select fdoc.FaultDayOccurrenceId, fdoc.FaultOccurrenceId, fdoc.StartDTTM, fdoc.EndDTTM, fdoc.EnergyCost 
		from Asset_Diag.dbo.FaultDayOccurrence fdoc
	inner join Asset_Diag.dbo.FaultOccurrence foc on foc.FaultOccurrenceID = fdoc.FaultOccurrenceId
	inner join Asset_Diag.dbo.Faults ft on foc.FaultId = ft.FaultID
	inner join Asset_Diag.dbo.Asset ass on ft.AssetID = ass.AssetID
	inner join Asset_Diag.dbo.SiteAsset sass on sass.AssetID = ass.AssetID
	where 
		(ft.StartDTTM between @StartDate and @EndDate AND 
		 ft.EndDTTM between @StartDate AND @EndDate) 
	and sass.SiteID in (select SiteID from Asset_Diag.dbo.Site where Name = @SiteName)
END
GO

EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractFaultDayOccurrenceAllSite'
GO

CREATE PROCEDURE [dbo].ExtractFaultDayOccurrenceAllSite(@SiteName nvarchar(200))
AS
BEGIN
	SET NOCOUNT ON
	select fdoc.FaultDayOccurrenceId, fdoc.FaultOccurrenceId, fdoc.StartDTTM, fdoc.EndDTTM, fdoc.EnergyCost 
		from Asset_Diag.dbo.FaultDayOccurrence fdoc
	inner join Asset_Diag.dbo.FaultOccurrence foc on foc.FaultOccurrenceID = fdoc.FaultOccurrenceId
	inner join Asset_Diag.dbo.Faults ft on foc.FaultId = ft.FaultID
	inner join Asset_Diag.dbo.Asset ass on ft.AssetID = ass.AssetID
	inner join Asset_Diag.dbo.SiteAsset sass on sass.AssetID = ass.AssetID
	where sass.SiteID in (select SiteID from Asset_Diag.dbo.Site where Name = @SiteName)
END
GO

EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractFaultDayOccurrenceAllDate'
GO

CREATE PROCEDURE [dbo].ExtractFaultDayOccurrenceAllDate(@StartDate datetime, @EndDate datetime)
AS
BEGIN
	SET NOCOUNT ON
	select fdoc.FaultDayOccurrenceId, fdoc.FaultOccurrenceId, fdoc.StartDTTM, fdoc.EndDTTM, fdoc.EnergyCost 
		from Asset_Diag.dbo.FaultDayOccurrence fdoc
	inner join Asset_Diag.dbo.FaultOccurrence foc on foc.FaultOccurrenceID = fdoc.FaultOccurrenceId
	inner join Asset_Diag.dbo.Faults ft on foc.FaultId = ft.FaultID
	where 
		(ft.StartDTTM between @StartDate and @EndDate AND 
		 ft.EndDTTM between @StartDate AND @EndDate) 
END
GO

EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractFaultDayOccurrenceAll'
GO

CREATE PROCEDURE [dbo].ExtractFaultDayOccurrenceAll
AS
BEGIN
	SET NOCOUNT ON
	select fdoc.FaultDayOccurrenceId, fdoc.FaultOccurrenceId, fdoc.StartDTTM, fdoc.EndDTTM, fdoc.EnergyCost 
		from Asset_Diag.dbo.FaultDayOccurrence fdoc
END
GO

-------------------------------------------------------------------------------
--FaultOccurrencePoint 
-------------------------------------------------------------------------------
EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractFaultOccurrencePoint'
GO

CREATE PROCEDURE [dbo].ExtractFaultOccurrencePoint(@SiteName nvarchar(200), @StartDate datetime, @EndDate datetime)
AS
BEGIN
	SET NOCOUNT ON
	select fop.* from Asset_Diag.dbo.FaultOccurrencePoint fop
	inner join Asset_Diag.dbo.FaultDayOccurrence fdoc on fdoc.FaultDayOccurrenceId = fop.FaultDayOccurrenceID
	inner join Asset_Diag.dbo.FaultOccurrence foc on foc.FaultOccurrenceID = fdoc.FaultOccurrenceId
	inner join Asset_Diag.dbo.Faults ft on foc.FaultId = ft.FaultID
	inner join Asset_Diag.dbo.Asset ass on ft.AssetID = ass.AssetID
	inner join Asset_Diag.dbo.SiteAsset sass on sass.AssetID = ass.AssetID
	where 
		(ft.StartDTTM between @StartDate and @EndDate AND 
		 ft.EndDTTM between @StartDate AND @EndDate) 
	and sass.SiteID in (select SiteID from Asset_Diag.dbo.Site where Name = @SiteName)
END
GO

EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractFaultOccurrencePointAllSite'
GO

CREATE PROCEDURE [dbo].ExtractFaultOccurrencePointAllSite(@SiteName nvarchar(200))
AS
BEGIN
	SET NOCOUNT ON
	select fop.* from Asset_Diag.dbo.FaultOccurrencePoint fop
	inner join Asset_Diag.dbo.FaultDayOccurrence fdoc on fdoc.FaultDayOccurrenceId = fop.FaultDayOccurrenceID
	inner join Asset_Diag.dbo.FaultOccurrence foc on foc.FaultOccurrenceID = fdoc.FaultOccurrenceId
	inner join Asset_Diag.dbo.Faults ft on foc.FaultId = ft.FaultID
	inner join Asset_Diag.dbo.Asset ass on ft.AssetID = ass.AssetID
	inner join Asset_Diag.dbo.SiteAsset sass on sass.AssetID = ass.AssetID
	where sass.SiteID in (select SiteID from Asset_Diag.dbo.Site where Name = @SiteName)
END
GO

EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractFaultOccurrencePointAllDate'
GO

CREATE PROCEDURE [dbo].ExtractFaultOccurrencePointAllDate(@StartDate datetime, @EndDate datetime)
AS
BEGIN
	SET NOCOUNT ON
	select fop.* from Asset_Diag.dbo.FaultOccurrencePoint fop
	inner join Asset_Diag.dbo.FaultDayOccurrence fdoc on fdoc.FaultDayOccurrenceId = fop.FaultDayOccurrenceID
	inner join Asset_Diag.dbo.FaultOccurrence foc on foc.FaultOccurrenceID = fdoc.FaultOccurrenceId
	inner join Asset_Diag.dbo.Faults ft on foc.FaultId = ft.FaultID
	inner join Asset_Diag.dbo.Asset ass on ft.AssetID = ass.AssetID
	inner join Asset_Diag.dbo.SiteAsset sass on sass.AssetID = ass.AssetID
	where 
		(ft.StartDTTM between @StartDate and @EndDate AND 
		 ft.EndDTTM between @StartDate AND @EndDate) 
END
GO

EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractFaultOccurrencePointAll'
GO

CREATE PROCEDURE [dbo].ExtractFaultOccurrencePointAll
AS
BEGIN
	SET NOCOUNT ON
	select fop.* from Asset_Diag.dbo.FaultOccurrencePoint fop
END
GO

-------------------------------------------------------------------------------
--FaultComment
-------------------------------------------------------------------------------
EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractFaultComment'
GO

CREATE PROCEDURE [dbo].ExtractFaultComment(@SiteName nvarchar(200), @StartDate datetime, @EndDate datetime)
AS
BEGIN
	SET NOCOUNT ON
	select fc.* from Asset_Diag.dbo.FaultComment fc
	inner join Asset_Diag.dbo.Faults ft on ft.FaultID = fc.FaultID
	inner join Asset_Diag.dbo.Asset ass on ft.AssetID = ass.AssetID
	inner join Asset_Diag.dbo.SiteAsset sass on sass.AssetID = ass.AssetID
	where 
		(ft.StartDTTM between @StartDate and @EndDate AND 
		 ft.EndDTTM between @StartDate AND @EndDate) 
	and sass.SiteID in (select SiteID from Asset_Diag.dbo.Site where Name = @SiteName)
END
GO

EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractFaultCommentAllSite'
GO

CREATE PROCEDURE [dbo].ExtractFaultCommentAllSite(@SiteName nvarchar(200))
AS
BEGIN
	SET NOCOUNT ON
	select fc.* from Asset_Diag.dbo.FaultComment fc
	inner join Asset_Diag.dbo.Faults ft on ft.FaultID = fc.FaultID
	inner join Asset_Diag.dbo.Asset ass on ft.AssetID = ass.AssetID
	inner join Asset_Diag.dbo.SiteAsset sass on sass.AssetID = ass.AssetID
	where sass.SiteID in (select SiteID from Asset_Diag.dbo.Site where Name = @SiteName)
END
GO

EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractFaultCommentAllDate'
GO

CREATE PROCEDURE [dbo].ExtractFaultCommentAllDate(@StartDate datetime, @EndDate datetime)
AS
BEGIN
	SET NOCOUNT ON
	select fc.* from Asset_Diag.dbo.FaultComment fc
	inner join Asset_Diag.dbo.Faults ft on ft.FaultID = fc.FaultID
	where 
		(ft.StartDTTM between @StartDate and @EndDate AND 
		 ft.EndDTTM between @StartDate AND @EndDate) 
END
GO

EXEC [dbo].DropProcedureWithCheck N'dbo.ExtractFaultCommentAll'
GO

CREATE PROCEDURE [dbo].ExtractFaultCommentAll
AS
BEGIN
	SET NOCOUNT ON
	select fc.* from Asset_Diag.dbo.FaultComment fc
END
GO
-------------------------------------------------------------------------------
--Physical point data
-------------------------------------------------------------------------------
EXEC dbo.DropProcedureWithCheck N'dbo.ExtractPhysicalPointData'
GO

CREATE PROCEDURE [dbo].ExtractPhysicalPointData(@SiteName nvarchar(200), @StartDate datetime, @EndDate datetime)
AS
BEGIN
	SET NOCOUNT ON
	;with deduppts
	AS
	(
		select pdat.PointMapID, PointDTTM, PointValue, DataQuality, CreatedDTTM, EndDTTM, ROW_NUMBER() OVER (PARTITION BY pdat.PointMapID, pdat.PointDTTM ORDER BY pdat.PointDTTM ASC) AS RN
		from Asset_Diag.dbo.PointDataMapLatest pdat
			inner join Asset_Diag.dbo.PointBase pb on pb.PointMapID = pdat.PointMapID
			inner join Asset_Diag.dbo.Asset ass on pb.AssetID = ass.AssetID
			inner join Asset_Diag.dbo.SiteAsset sass on sass.AssetID = ass.AssetID
			inner join Asset_Diag.dbo.PointType ptyp on ptyp.PointTypeID = pb.PointTypeID
		where ptyp.Type IN ('Physical','ManualInputMonthly')
			and pdat.PointDTTM between @StartDate and @EndDate
			and sass.SiteID in (select SiteID from Asset_Diag.dbo.Site where Name = @SiteName)
	)
	select PointMapID, PointDTTM, PointValue, DataQuality, CreatedDTTM, EndDTTM from deduppts where RN = 1
	order by PointMapID ASC, PointDTTM ASC
END
GO

-------------------------------------------------------------------------------
-- CAUTION: Will return a very large data set!!!!
-------------------------------------------------------------------------------
EXEC dbo.DropProcedureWithCheck N'dbo.ExtractPhysicalPointDataAllSite'
GO

CREATE PROCEDURE [dbo].ExtractPhysicalPointDataAllSite(@SiteName nvarchar(200))
AS
BEGIN
	SET NOCOUNT ON
	;with deduppts
	AS
	(
		select pdat.PointMapID, PointDTTM, PointValue, DataQuality, CreatedDTTM, EndDTTM, ROW_NUMBER() OVER (PARTITION BY pdat.PointMapID, pdat.PointDTTM ORDER BY pdat.PointDTTM ASC) AS RN
		from Asset_Diag.dbo.PointDataMapLatest pdat
			inner join Asset_Diag.dbo.PointBase pb on pb.PointMapID = pdat.PointMapID
			inner join Asset_Diag.dbo.Asset ass on pb.AssetID = ass.AssetID
			inner join Asset_Diag.dbo.SiteAsset sass on sass.AssetID = ass.AssetID
			inner join Asset_Diag.dbo.PointType ptyp on ptyp.PointTypeID = pb.PointTypeID
		where ptyp.Type IN ('Physical','ManualInputMonthly')
			and sass.SiteID in (select SiteID from Asset_Diag.dbo.Site where Name = @SiteName)
	)
	select PointMapID, PointDTTM, PointValue, DataQuality, CreatedDTTM, EndDTTM from deduppts where RN = 1
	order by PointMapID ASC, PointDTTM ASC
END
GO

EXEC dbo.DropProcedureWithCheck N'dbo.ExtractPhysicalPointDataAllDate'
GO

CREATE PROCEDURE [dbo].ExtractPhysicalPointDataAllDate(@StartDate datetime, @EndDate datetime)
AS
BEGIN
	SET NOCOUNT ON
	;with deduppts
	AS
	(
		select pdat.PointMapID, PointDTTM, PointValue, DataQuality, CreatedDTTM, EndDTTM, ROW_NUMBER() OVER (PARTITION BY pdat.PointMapID, pdat.PointDTTM ORDER BY pdat.PointDTTM ASC) AS RN
		from Asset_Diag.dbo.PointDataMapLatest pdat
			inner join Asset_Diag.dbo.PointBase pb on pb.PointMapID = pdat.PointMapID
			inner join Asset_Diag.dbo.PointType ptyp on ptyp.PointTypeID = pb.PointTypeID
		where ptyp.Type IN ('Physical','ManualInputMonthly')
			and pdat.PointDTTM between @StartDate and @EndDate
	)
	select PointMapID, PointDTTM, PointValue, DataQuality, CreatedDTTM, EndDTTM from deduppts where RN = 1
	order by PointMapID ASC, PointDTTM ASC
END
GO

-------------------------------------------------------------------------------
-- CAUTION: Will return a very large data set!!!!
-------------------------------------------------------------------------------
EXEC dbo.DropProcedureWithCheck N'dbo.ExtractPhysicalPointDataAll'
GO

CREATE PROCEDURE [dbo].ExtractPhysicalPointDataAll
AS
BEGIN
	SET NOCOUNT ON
	;with deduppts
	AS
	(
		select pdat.PointMapID, PointDTTM, PointValue, DataQuality, CreatedDTTM, EndDTTM, ROW_NUMBER() OVER (PARTITION BY pdat.PointMapID, pdat.PointDTTM ORDER BY pdat.PointDTTM ASC) AS RN
		from Asset_Diag.dbo.PointDataMapLatest pdat
			inner join Asset_Diag.dbo.PointBase pb on pb.PointMapID = pdat.PointMapID
			inner join Asset_Diag.dbo.PointType ptyp on ptyp.PointTypeID = pb.PointTypeID
		where ptyp.Type IN ('Physical','ManualInputMonthly')
	)
	select PointMapID, PointDTTM, PointValue, DataQuality, CreatedDTTM, EndDTTM from deduppts where RN = 1
	order by PointMapID ASC, PointDTTM ASC
END
GO
-------------------------------------------------------------------------------
--UserInput point data (still based on PointID, use PointBase to get PointMapID)
-------------------------------------------------------------------------------
EXEC dbo.DropProcedureWithCheck N'dbo.ExtractUserInputPointData'
GO

CREATE PROCEDURE [dbo].ExtractUserInputPointData(@SiteName nvarchar(200), @StartDate datetime, @EndDate datetime)
AS
BEGIN
	SET NOCOUNT ON
	
	select pui.* from Asset_Diag.dbo.PointData_UserInput pui
		inner join Asset_Diag.dbo.PointBase pb on pb.PointID = pui.PointID
		inner join Asset_Diag.dbo.Asset ass on pb.AssetID = ass.AssetID
		inner join Asset_Diag.dbo.SiteAsset sass on sass.AssetID = ass.AssetID
	where 
		pui.PointDTTM between @StartDate and @EndDate
		and sass.SiteID in (select SiteID from Asset_Diag.dbo.Site where Name = @SiteName)
END
GO

EXEC dbo.DropProcedureWithCheck N'dbo.ExtractUserInputPointDataAllSite'
GO

CREATE PROCEDURE [dbo].ExtractUserInputPointDataAllSite(@SiteName nvarchar(200))
AS
BEGIN
	SET NOCOUNT ON
	
	select pui.* from Asset_Diag.dbo.PointData_UserInput pui
		inner join Asset_Diag.dbo.PointBase pb on pb.PointID = pui.PointID
		inner join Asset_Diag.dbo.Asset ass on pb.AssetID = ass.AssetID
		inner join Asset_Diag.dbo.SiteAsset sass on sass.AssetID = ass.AssetID
	where 
		sass.SiteID in (select SiteID from Asset_Diag.dbo.Site where Name = @SiteName)
END
GO

EXEC dbo.DropProcedureWithCheck N'dbo.ExtractUserInputPointDataAllDate'
GO

CREATE PROCEDURE [dbo].ExtractUserInputPointDataAllDate(@StartDate datetime, @EndDate datetime)
AS
BEGIN
	SET NOCOUNT ON
	
	select pui.* from Asset_Diag.dbo.PointData_UserInput pui
	where 
		pui.PointDTTM between @StartDate and @EndDate
END
GO

EXEC dbo.DropProcedureWithCheck N'dbo.ExtractUserInputPointDataAll'
GO

CREATE PROCEDURE [dbo].ExtractUserInputPointDataAll
AS
BEGIN
	SET NOCOUNT ON
	select pui.* from Asset_Diag.dbo.PointData_UserInput pui
END
GO

-------------------------------------------------------------------------------
--Calculated Point data
-------------------------------------------------------------------------------
EXEC dbo.DropProcedureWithCheck N'dbo.ExtractCalculatedPointData'
GO

CREATE PROCEDURE [dbo].ExtractCalculatedPointData(@SiteName nvarchar(200), @StartDate datetime, @EndDate datetime)
AS
BEGIN
	SET NOCOUNT ON
	;with deduppts
	AS
	(
		select pdat.PointMapID, PointDTTM, PointValue, DataQuality, CreatedDTTM, EndDTTM, ROW_NUMBER() OVER (PARTITION BY pdat.PointMapID, pdat.PointDTTM ORDER BY pdat.PointDTTM ASC) AS RN
		from Asset_Diag.dbo.PointDataMapLatest pdat
			inner join Asset_Diag.dbo.PointBase pb on pb.PointMapID = pdat.PointMapID
			inner join Asset_Diag.dbo.Asset ass on pb.AssetID = ass.AssetID
			inner join Asset_Diag.dbo.SiteAsset sass on sass.AssetID = ass.AssetID
			inner join Asset_Diag.dbo.PointType ptyp on ptyp.PointTypeID = pb.PointTypeID
		where 
			ptyp.Type in ('EnergyCalculated', 'DailyAggregation', 'MonthlyExportCalc') 
			and pdat.PointDTTM between @StartDate and @EndDate
			and sass.SiteID in (select SiteID from Asset_Diag.dbo.Site where Name = @SiteName)
	)
	select PointMapID, PointDTTM, PointValue, DataQuality, CreatedDTTM, EndDTTM from deduppts where RN = 1
	order by PointMapID ASC, PointDTTM ASC
END
GO

-------------------------------------------------------------------------------
-- CAUTION: Will return a very large data set!!!!
-------------------------------------------------------------------------------
EXEC dbo.DropProcedureWithCheck N'dbo.ExtractCalculatedPointDataAllSite'
GO

CREATE PROCEDURE [dbo].ExtractCalculatedPointDataAllSite(@SiteName nvarchar(200))
AS
BEGIN
	SET NOCOUNT ON
	;with deduppts
	AS
	(
		select pdat.PointMapID, PointDTTM, PointValue, DataQuality, CreatedDTTM, EndDTTM, ROW_NUMBER() OVER (PARTITION BY pdat.PointMapID, pdat.PointDTTM ORDER BY pdat.PointDTTM ASC) AS RN
		from Asset_Diag.dbo.PointDataMapLatest pdat
			inner join Asset_Diag.dbo.PointBase pb on pb.PointMapID = pdat.PointMapID
			inner join Asset_Diag.dbo.Asset ass on pb.AssetID = ass.AssetID
			inner join Asset_Diag.dbo.SiteAsset sass on sass.AssetID = ass.AssetID
			inner join Asset_Diag.dbo.PointType ptyp on ptyp.PointTypeID = pb.PointTypeID
		where 
			ptyp.Type in ('EnergyCalculated', 'DailyAggregation', 'MonthlyExportCalc') 
			and sass.SiteID in (select SiteID from Asset_Diag.dbo.Site where Name = @SiteName)
	)
	select PointMapID, PointDTTM, PointValue, DataQuality, CreatedDTTM, EndDTTM from deduppts where RN = 1
	order by PointMapID ASC, PointDTTM ASC
END
GO

EXEC dbo.DropProcedureWithCheck N'dbo.ExtractCalculatedPointDataAllDate'
GO

CREATE PROCEDURE [dbo].ExtractCalculatedPointDataAllDate(@StartDate datetime, @EndDate datetime)
AS
BEGIN
	SET NOCOUNT ON
	;with deduppts
	AS
	(
		select pdat.PointMapID, PointDTTM, PointValue, DataQuality, CreatedDTTM, EndDTTM, ROW_NUMBER() OVER (PARTITION BY pdat.PointMapID, pdat.PointDTTM ORDER BY pdat.PointDTTM ASC) AS RN
		from Asset_Diag.dbo.PointDataMapLatest pdat
			inner join Asset_Diag.dbo.PointBase pb on pb.PointMapID = pdat.PointMapID
			inner join Asset_Diag.dbo.PointType ptyp on ptyp.PointTypeID = pb.PointTypeID
		where 
			ptyp.Type in ('EnergyCalculated', 'DailyAggregation', 'MonthlyExportCalc') 
			and pdat.PointDTTM between @StartDate and @EndDate
	)
	select PointMapID, PointDTTM, PointValue, DataQuality, CreatedDTTM, EndDTTM from deduppts where RN = 1
	order by PointMapID ASC, PointDTTM ASC
END
GO

-------------------------------------------------------------------------------
-- CAUTION: Will return a very large data set!!!!
-------------------------------------------------------------------------------
EXEC dbo.DropProcedureWithCheck N'dbo.ExtractCalculatedPointDataAll'
GO

CREATE PROCEDURE [dbo].ExtractCalculatedPointDataAll
AS
BEGIN
	SET NOCOUNT ON
	;with deduppts
	AS
	(
		select pdat.PointMapID, PointDTTM, PointValue, DataQuality, CreatedDTTM, EndDTTM, ROW_NUMBER() OVER (PARTITION BY pdat.PointMapID, pdat.PointDTTM ORDER BY pdat.PointDTTM ASC) AS RN
		from Asset_Diag.dbo.PointDataMapLatest pdat
			inner join Asset_Diag.dbo.PointBase pb on pb.PointMapID = pdat.PointMapID
			inner join Asset_Diag.dbo.PointType ptyp on ptyp.PointTypeID = pb.PointTypeID
		where 
			ptyp.Type in ('EnergyCalculated', 'DailyAggregation', 'MonthlyExportCalc') 
	)
	select PointMapID, PointDTTM, PointValue, DataQuality, CreatedDTTM, EndDTTM from deduppts where RN = 1
	order by PointMapID ASC, PointDTTM ASC
END
GO

-------------------------------------------------------------------------------
EXEC dbo.DropFunctionWithCheck N'dbo.ComputeExtractString'
GO

CREATE FUNCTION [dbo].GenExStr(@UseSite int, @UseDate int, @SiteName nvarchar(300), @StartDate nvarchar(300), @EndDate nvarchar(300))
RETURNS NVARCHAR(max)
WITH EXECUTE AS CALLER
AS
BEGIN
	DECLARE @ExStr NVARCHAR(max)

	IF @UseSite = 1
	BEGIN
		IF @UseDate = 1
		BEGIN
			SET @ExStr = N' N''' + @SiteName + ''', ''' + @StartDate + ''', ''' + @EndDate + ''''
		END
		ELSE
		BEGIN
			SET @ExStr = N'AllSite N''' + @SiteName + ''''
		END
	END
	ELSE
	BEGIN
		IF @UseDate = 1
		BEGIN
			SET @ExStr = N'AllDate N''' + @StartDate + ''', ''' + @EndDate + ''''
		END
		ELSE
		BEGIN
			SET @ExStr = N'All'
		END
	END
	
	RETURN @ExStr
END
	

-------------------------------------------------------------------------------
-- TODO
-- Need Weather Data
-- Weather tables
-- 

-------------------------------------------------------------------------------
-- Main BCP Output Utility Function
-- NOTE: Will output a BCP extract to a location on the SQL server box - 
--       *** THIS DOES NOT PUT A FILE ON YOUR LOCAL MACHINE ***
--       ***THIS IS WHY THE PATH YOU SPECIFY ABOVE MUST EXIST ON THE SERVER***
-- NOTE: master..xpcmdshell must be enabled before this can be run
--       It is enabled by default in production...it may or may not be
--       enabled in staging.  
-- NOTE: To check if it is enabled, execute the following query (return of 1 
--           means it is enabled):
--
--       SELECT CONVERT(INT, ISNULL(value, value_in_use)) AS config_value
--       FROM  sys.configurations
--       WHERE  name = N'xp_cmdshell' 
--
--             
-- NOTE: To enable, execute the following statement block:
--
--       RECONFIGURE
--       GO
--       EXEC sp_configure 'xp_cmdshell', 1
--       GO
--       RECONFIGURE
--       GO
--
-- NOTE: IF AND ONLY IF it was disabled prior to running this script, please 
--       disable the functionality by executing the following statement block:
--
--       RECONFIGURE
--       GO
--       EXEC sp_configure 'xp_cmdshell', 0
--       GO
--       RECONFIGURE
--       GO
-------------------------------------------------------------------------------
EXEC [dbo].[DropProcedureWithCheck] N'dbo.BCPOutput'
GO

CREATE PROCEDURE [dbo].BCPOutput(@sqlCmd nvarchar(200) , @fileName nvarchar(200), @tableName nvarchar(200))
AS
BEGIN
	DECLARE @SQL varchar(2000)
	SELECT @SQL = 'bcp "exec ' + @sqlCmd + '" queryout ' + @fileName + '.dat -c -T -S' + @@SERVERNAME 
	EXEC master..xp_cmdshell @SQL

	SELECT @SQL = 'bcp ' + @tableName + ' format nul -c -x -f ' + @fileName + '-fmt.xml -T -S' + @@SERVERNAME 
	EXEC master..xp_cmdshell @SQL
END
GO	

-------------------------------------------------------------------------------
-- MAIN 
-- See the top of this SQL file to set the site name, date ranges, and
-- server local path
-- Single Asset Tree Site Extraction Script
-------------------------------------------------------------------------------
BEGIN	
	DECLARE @GlobSiteName nvarchar(300)
	DECLARE @GlobStartDate nvarchar(300)
	DECLARE @GlobEndDate nvarchar(300)
	DECLARE @EnvBasePath nvarchar(20)
	DECLARE @ExtractAllAsset int
	DECLARE @ExtractAllFault int
	DECLARE @ExtractAllPhysical int
	DECLARE @ExtractAllCalculated int
	-- only applies for Faults, Physical, and Calculated extracts
	--  when the all flag for that type is 0
	DECLARE @FilterSite int 
	-- only applies for Faults, Physical, and Calculated extracts
	--  when the all flag for that type is 0
	DECLARE @FilterDate int 
	
	------------------------------------------
	--SET YOUR SITE AND DATE VARIABLES HERE
	------------------------------------------
	SET @GlobSiteName = 'Parkway Center 3'
	SET @GlobStartDate = '2015-01-01'
	SET @GlobEndDate   = '2015-01-31'
	SET @EnvBasePath = 'z:\sitedat\P3'
	SET @ExtractAllAsset = 1
	SET @ExtractAllFault = 1
	SET @ExtractAllPhysical = 0
	SET @ExtractAllCalculated = 0
	SET @FilterSite = 1
	SET @FilterDate = 1

	------------------------------------------
	-- DONT EDIT BEYOND THIS POINT
	------------------------------------------
	DECLARE @SQL varchar(2000)
	DECLARE @SiteName nvarchar(300)
	DECLARE @StartDate nvarchar(300)
	DECLARE @EndDate nvarchar(300)
	DECLARE @BasePath nvarchar(20)
	DECLARE @FullPath nvarchar(300)
	SET @SiteName  = @GlobSiteName
	SET @StartDate = @GlobStartDate + ' 00:00:00.000'
	SET @EndDate   = @GlobEndDate + ' 23:59:59.000'
	SET @BasePath  = @EnvBasePath + '\\'
	
	DECLARE @FxnCall nvarchar(300)
	
	
	IF @ExtractAllAsset = 0
	BEGIN
		SET @FullPath = @BasePath + N'AssetTree\1_Client'
		SET @FxnCall = N'Asset_Diag.dbo.ExtractClient N''' + @SiteName + ''''
		EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.Client'
	END
	ELSE
	BEGIN
		SET @FullPath = @BasePath + N'AssetTree\1_Client'
		SET @FxnCall = N'Asset_Diag.dbo.ExtractClientAll'
		EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.Client'
	END
	
	IF @ExtractAllAsset = 0
	BEGIN
		SET @FullPath = @BasePath + N'AssetTree\2_Site'
		SET @FxnCall = N'Asset_Diag.dbo.ExtractSite N''' + @SiteName + ''''
		EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.Site'	
	END
	ELSE
	BEGIN
		SET @FullPath = @BasePath + N'AssetTree\2_Site'
		SET @FxnCall = N'Asset_Diag.dbo.ExtractSiteAll'
		EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.Site'	
	END
	
	
	IF @ExtractAllAsset = 0
	BEGIN
		SET @FullPath = @BasePath + N'AssetTree\3_Asset'
		SET @FxnCall = N'Asset_Diag.dbo.ExtractAsset N''' + @SiteName + ''''
		EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.Asset'	
	END
	ELSE
	BEGIN
		SET @FullPath = @BasePath + N'AssetTree\3_Asset'
		SET @FxnCall = N'Asset_Diag.dbo.ExtractAssetAll'
		EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.Asset'	
	
	IF @ExtractAllAsset = 0
	BEGIN
		SET @FullPath = @BasePath + N'AssetTree\4_AttributeData'
		SET @FxnCall = N'Asset_Diag.dbo.ExtractAttributeData N''' + @SiteName + ''''
		EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.AttributeData'	
	END
	ELSE
	BEGIN
		SET @FullPath = @BasePath + N'AssetTree\4_AttributeData'
		SET @FxnCall = N'Asset_Diag.dbo.ExtractAttributeDataAll'
		EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.AttributeData'	
	END
	
	IF @ExtractAllAsset = 0
	BEGIN
		SET @FullPath = @BasePath + N'AssetTree\5_SiteAsset'
		SET @FxnCall = N'Asset_Diag.dbo.ExtractSiteAsset N''' + @SiteName + ''''
		EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.SiteAsset'	
	END
	ELSE
	BEGIN
		SET @FullPath = @BasePath + N'AssetTree\5_SiteAsset'
		SET @FxnCall = N'Asset_Diag.dbo.ExtractSiteAssetAll'
		EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.SiteAsset'	
	END

	IF @ExtractAllAsset = 0
	BEGIN
		SET @FullPath = @BasePath + N'AssetTree\6_PointMap'
		SET @FxnCall = N'Asset_Diag.dbo.ExtractPointMap N''' + @SiteName + ''''
		EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.PointMap'	
	END
	ELSE
	BEGIN
		SET @FullPath = @BasePath + N'AssetTree\6_PointMap'
		SET @FxnCall = N'Asset_Diag.dbo.ExtractPointMapAll'
		EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.PointMap'	
	END

	IF @ExtractAllAsset = 0
	BEGIN
		SET @FullPath = @BasePath + N'AssetTree\7_BasPoint'
		SET @FxnCall = N'Asset_Diag.dbo.ExtractBasPoint N''' + @SiteName + ''''
		EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.BasPoint'	
	END
	ELSE
	BEGIN
		SET @FullPath = @BasePath + N'AssetTree\7_BasPoint'
		SET @FxnCall = N'Asset_Diag.dbo.ExtractBasPointAll'
		EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.BasPoint'	
	END

	IF @ExtractAllAsset = 0
	BEGIN
		SET @FullPath = @BasePath + N'AssetTree\8_PointBase'
		SET @FxnCall = N'Asset_Diag.dbo.ExtractPointBase N''' + @SiteName + ''''
		EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.PointBase'	
	END
	ELSE
	BEGIN
		SET @FullPath = @BasePath + N'AssetTree\8_PointBase'
		SET @FxnCall = N'Asset_Diag.dbo.ExtractPointBaseAll'
		EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.PointBase'	
	END

	SET @FullPath = @BasePath + N'AssetTree\9_Users'
	SET @FxnCall = N'Asset_Diag.dbo.ExtractUsers'
	EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.pf.Users'	
	
	SET @FullPath = @BasePath + N'AssetTree\10_Role'
	SET @FxnCall = N'Asset_Diag.dbo.ExtractRole'
	EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.pf.Role'	

	SET @FullPath = @BasePath + N'AssetTree\11_UserClient'
	SET @FxnCall = N'Asset_Diag.dbo.ExtractUserClient'
	EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.pf.UserClient'	

	SET @FullPath = @BasePath + N'AssetTree\12_UserRole'
	SET @FxnCall = N'Asset_Diag.dbo.ExtractUserRole'
	EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.pf.UserRole'	

	IF @ExtractAllAsset = 0
	BEGIN
		SET @FullPath = @BasePath + N'AssetTree\13_PointMapDetails'
		SET @FxnCall = N'Asset_Diag.dbo.ExtractPointMapDetails N''' + @SiteName + ''''
		EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.PointMapDetails'	
	END
	ELSE
	BEGIN
		SET @FullPath = @BasePath + N'AssetTree\13_PointMapDetails'
		SET @FxnCall = N'Asset_Diag.dbo.ExtractPointMapDetailsAll'
		EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.PointMapDetails'	
	END
	
	IF @ExtractAllAsset = 0
	BEGIN
		SET @FullPath = @BasePath + N'AssetTree\14_AddressSimple'
		SET @FxnCall = N'Asset_Diag.dbo.ExtractAddressSimple N''' + @SiteName + ''''
		EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.AddressSimple'	
	END
	ELSE
	BEGIN
		SET @FullPath = @BasePath + N'AssetTree\14_AddressSimple'
		SET @FxnCall = N'Asset_Diag.dbo.ExtractAddressSimpleAll N''' + @SiteName + ''''
		EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.AddressSimple'	
	END

	IF @ExtractAllAsset = 0
	BEGIN
		SET @FullPath = @BasePath + N'AssetTree\15_SiteAddressSimple'
		SET @FxnCall = N'Asset_Diag.dbo.ExtractSiteAddressSimple N''' + @SiteName + ''''
		EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.SiteAddressSimple'		
	END
	ELSE
	BEGIN
		SET @FullPath = @BasePath + N'AssetTree\15_SiteAddressSimple'
		SET @FxnCall = N'Asset_Diag.dbo.ExtractSiteAddressSimpleAll'
		EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.SiteAddressSimple'		
	END

	IF @ExtractAllAsset = 0
	BEGIN
		SET @FullPath = @BasePath + N'AssetTree\16_Schedule'
		SET @FxnCall = N'Asset_Diag.dbo.ExtractSchedule N''' + @SiteName + ''''
		EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.Schedule'		
	END
	ELSE
	BEGIN
		SET @FullPath = @BasePath + N'AssetTree\16_Schedule'
		SET @FxnCall = N'Asset_Diag.dbo.ExtractScheduleAll'
		EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.Schedule'		
	END

	IF @ExtractAllAsset = 0
	BEGIN
		SET @FullPath = @BasePath + N'AssetTree\17_ScheduleChangeLog'
		SET @FxnCall = N'Asset_Diag.dbo.ExtractScheduleChangeLog N''' + @SiteName + ''''
		EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.ScheduleChangeLog'		
	END
	ELSE
	BEGIN
		SET @FullPath = @BasePath + N'AssetTree\17_ScheduleChangeLog'
		SET @FxnCall = N'Asset_Diag.dbo.ExtractScheduleChangeLogAll'
		EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.ScheduleChangeLog'		
	END

	IF @ExtractAllAsset = 0
	BEGIN
		SET @FullPath = @BasePath + N'AssetTree\18_ScheduleDetail'
		SET @FxnCall = N'Asset_Diag.dbo.ExtractScheduleDetail N''' + @SiteName + ''''
		EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.ScheduleDetail'		
	END
	ELSE
	BEGIN
		SET @FullPath = @BasePath + N'AssetTree\18_ScheduleDetail'
		SET @FxnCall = N'Asset_Diag.dbo.ExtractScheduleDetailAll'
		EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.ScheduleDetail'		
	END

	IF @ExtractAllAsset = 0
	BEGIN
		SET @FullPath = @BasePath + N'AssetTree\19_ScheduleDetailChangeLog'
		SET @FxnCall = N'Asset_Diag.dbo.ExtractScheduleDetailChangeLog N''' + @SiteName + ''''
		EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.ScheduleDetailChangeLog'		
	END
	ELSE
	BEGIN
		SET @FullPath = @BasePath + N'AssetTree\19_ScheduleDetailChangeLog'
		SET @FxnCall = N'Asset_Diag.dbo.ExtractScheduleDetailChangeLogAll'
		EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.ScheduleDetailChangeLog'		
	END

	IF @ExtractAllAsset = 0
	BEGIN
		SET @FullPath = @BasePath + N'AssetTree\20_ScheduleException'
		SET @FxnCall = N'Asset_Diag.dbo.ExtractScheduleException N''' + @SiteName + ''''
		EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.ScheduleException'		
	END
	ELSE
	BEGIN
		SET @FullPath = @BasePath + N'AssetTree\20_ScheduleException'
		SET @FxnCall = N'Asset_Diag.dbo.ExtractScheduleExceptionAll'
		EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.ScheduleException'		
	END
	
	IF @ExtractAllAsset = 0
	BEGIN
		SET @FullPath = @BasePath + N'AssetTree\21_ScheduleExceptionChangeLog'
		SET @FxnCall = N'Asset_Diag.dbo.ExtractScheduleExceptionChangeLog N''' + @SiteName + ''''
		EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.ScheduleExceptionChangeLog'		
	END
	ELSE
	BEGIN
		SET @FullPath = @BasePath + N'AssetTree\21_ScheduleExceptionChangeLog'
		SET @FxnCall = N'Asset_Diag.dbo.ExtractScheduleExceptionChangeLogAll'
		EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.ScheduleExceptionChangeLog'		
	END

	SET @FullPath = @BasePath + N'AssetTree\22_ScheduleModes'
	SET @FxnCall = N'Asset_Diag.dbo.ExtractScheduleModes'
	EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.ScheduleModes'		

	SET @FullPath = @BasePath + N'AssetTree\23_ScheduleRecurrenceDays'
	SET @FxnCall = N'Asset_Diag.dbo.ExtractScheduleRecurrenceDays'
	EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.ScheduleRecurrenceDays'		

	SET @FullPath = @BasePath + N'AssetTree\24_ScheduleRecurrenceFrequency'
	SET @FxnCall = N'Asset_Diag.dbo.ExtractScheduleRecurrenceFrequency'
	EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.ScheduleRecurrenceFrequency'		

	SET @FullPath = @BasePath + N'AssetTree\25_ScheduleRecurrenceMonth'
	SET @FxnCall = N'Asset_Diag.dbo.ExtractScheduleRecurrenceMonth'
	EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.ScheduleRecurrenceMonth'		

	SET @FullPath = @BasePath + N'AssetTree\26_ScheduleRecurType'
	SET @FxnCall = N'Asset_Diag.dbo.ExtractScheduleRecurType'
	EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.ScheduleRecurType'			
	
	IF @ExtractAllAsset = 0
	BEGIN
		SET @FullPath = @BasePath + N'AssetTree\27_AssetSchedule'
		SET @FxnCall = N'Asset_Diag.dbo.ExtractAssetSchedule N''' + @SiteName + ''''
		EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.AssetSchedule'		
	END
	ELSE
	BEGIN
		SET @FullPath = @BasePath + N'AssetTree\27_AssetSchedule'
		SET @FxnCall = N'Asset_Diag.dbo.ExtractAssetScheduleAll'
		EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.AssetSchedule'		
	END
	
	-- Faults
	
	IF @ExtractAllFault = 0
	BEGIN
		SET @FullPath = @BasePath + N'FaultData\1_Faults'
		SET @FxnCall = N'Asset_Diag.dbo.ExtractFaults' + [dbo].GenExStr(@FilterSite, @FilterDate, @SiteName, @StartDate, @EndDate) 
		EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.Faults'	
	END
	ELSE
	BEGIN
		SET @FullPath = @BasePath + N'FaultData\1_Faults'
		SET @FxnCall = N'Asset_Diag.dbo.ExtractFaultsAll'
		EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.Faults'	
	END
	
	--FaultOccurrence
	
	IF @ExtractAllFault = 0
	BEGIN
		SET @FullPath = @BasePath + N'FaultData\2_FaultOccurrence'
		SET @FxnCall = N'Asset_Diag.dbo.ExtractFaultOccurrence' + [dbo].GenExStr(@FilterSite, @FilterDate, @SiteName, @StartDate, @EndDate)
		EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.FaultOccurrence'	
	END
	ELSE
	BEGIN
		SET @FullPath = @BasePath + N'FaultData\2_FaultOccurrence'
		SET @FxnCall = N'Asset_Diag.dbo.ExtractFaultOccurrenceAll'
		EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.FaultOccurrence'	
	END

	--FaultDayOccurrence
	IF @ExtractAllFault = 0
	BEGIN
		SET @FullPath = @BasePath + N'FaultData\3_FaultDayOccurrence'
		SET @FxnCall = N'Asset_Diag.dbo.ExtractFaultDayOccurrence' + [dbo].GenExStr(@FilterSite, @FilterDate, @SiteName, @StartDate, @EndDate)
		EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.FaultDayOccurrence'	
	END
	ELSE
	BEGIN
		SET @FullPath = @BasePath + N'FaultData\3_FaultDayOccurrence'
		SET @FxnCall = N'Asset_Diag.dbo.ExtractFaultDayOccurrenceAll'
		EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.FaultDayOccurrence'	
	END
	
	--FaultOccurrencePoint	
	IF @ExtractAllFault = 0
	BEGIN
		SET @FullPath = @BasePath + N'FaultData\4_FaultOccurrencePoint'
		SET @FxnCall = N'Asset_Diag.dbo.ExtractFaultOccurrencePoint' + [dbo].GenExStr(@FilterSite, @FilterDate, @SiteName, @StartDate, @EndDate)
		EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.FaultOccurrencePoint'	
	END
	ELSE
	BEGIN

	--FaultComment
	IF @ExtractAllFault = 0
	BEGIN
		SET @FullPath = @BasePath + N'FaultData\5_FaultComment'
		SET @FxnCall = N'Asset_Diag.dbo.ExtractFaultComment' + [dbo].GenExStr(@FilterSite, @FilterDate, @SiteName, @StartDate, @EndDate)
		EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.FaultComment'	
	END
	ELSE
	BEGIN
		SET @FullPath = @BasePath + N'FaultData\5_FaultComment'
		SET @FxnCall = N'Asset_Diag.dbo.ExtractFaultCommentAll'
		EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.FaultComment'	
	END

	--Physical - User Input Points
	IF @ExtractAllPhysical = 0
	BEGIN
		SET @FullPath = @BasePath + N'PhysicalPointData\1_UIPoints'
		SET @FxnCall = N'Asset_Diag.dbo.ExtractUserInputPointData' + [dbo].GenExStr(@FilterSite, @FilterDate, @SiteName, @StartDate, @EndDate)
		EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.PointData_UserInput'	
	END
	ELSE
	BEGIN
		SET @FullPath = @BasePath + N'PhysicalPointData\1_UIPoints'
		SET @FxnCall = N'Asset_Diag.dbo.ExtractUserInputPointDataAll'
		EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.PointData_UserInput'	
	END
		
	--Physical - Physical Point Data
	IF @ExtractAllPhysical = 0
	BEGIN
		SET @FullPath = @BasePath + N'PhysicalPointData\2_PhysicalPointData'
		SET @FxnCall = N'Asset_Diag.dbo.ExtractPhysicalPointData' + [dbo].GenExStr(@FilterSite, @FilterDate, @SiteName, @StartDate, @EndDate)
		EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.PointData_2015_01'	--Table reference here is for format purposes only
	END
	ELSE
	BEGIN
		SET @FullPath = @BasePath + N'PhysicalPointData\2_PhysicalPointData'
		SET @FxnCall = N'Asset_Diag.dbo.ExtractPhysicalPointDataAll'
		EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.PointData_2015_01'	--Table reference here is for format purposes only
	END

	--Calculated - Calculated Point Data		
	IF @ExtractAllCalculated = 0
	BEGIN
		SET @FullPath = @BasePath + N'CalculatedPointData\1_CalculatedPointData'
		SET @FxnCall = N'Asset_Diag.dbo.ExtractCalculatedPointData' + [dbo].GenExStr(@FilterSite, @FilterDate, @SiteName, @StartDate, @EndDate)
		EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.PointData_2015_01'	--Table reference here is for format purposes only
	END
	ELSE
	BEGIN
		SET @FullPath = @BasePath + N'CalculatedPointData\1_CalculatedPointData'
		SET @FxnCall = N'Asset_Diag.dbo.ExtractCalculatedPointDataAll'
		EXEC [dbo].BCPOutput @FxnCall, @FullPath, N'Asset_Diag.dbo.PointData_2015_01'	--Table reference here is for format purposes only
	END
	
END
GO

