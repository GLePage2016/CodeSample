-- Pawn transactional loading sequence script
-- Change Log
-- Name           Date            Change
-- Gerald LePage  04-10-2013      Initial version
-- Gerald LePage  04-24-2013      Added more comments

CREATE OR REPLACE FUNCTION GenStoreDataStats(
    p_store VARCHAR2, 
    p_sample_start TIMESTAMP WITH TIME ZONE, 
    p_sample_end TIMESTAMP WITH TIME ZONE,
    

DECLARE
   recdet_rec   CCSOWNER.RECEIPTDETAIL%ROWTYPE;
   sto_rec      CCSOWNER.STORE%ROWTYPE;
   v_sample_start TIMESTAMP(6) WITH TIME ZONE;
   v_sample_end   TIMESTAMP(6) WITH TIME ZONE;
   v_local_time   TIMESTAMP(6) WITH LOCAL TIME ZONE;
   v_trans_event  CCSOWNER.RECEIPTDETAIL.REF_EVENT%TYPE;
   v_trans_type   CCSOWNER.RECEIPTDETAIL.REF_TYPE%TYPE;
   v_sample_rate NUMBER(12,3);
   v_sample_rate_mins NUMBER(2);
   v_sample_size  NATURAL;
   v_curmin NUMBER(2);
   v_count  NUMBER(12);
   v_isbegin VARCHAR2(1);
   v_issamplebegin VARCHAR2(1);
   v_last_sample_min NUMBER(2);
   v_duration NUMBER(10);
   v_total NUMBER(12);
   v_sample_loans_second NUMBER(12,4);
   v_total_seconds NUMBER(12);
   v_total_loans_second NUMBER(12,4);
   v_start_month VARCHAR2(4);
   v_start_month_day VARCHAR2(4);
   v_start_year VARCHAR2(4);
   v_start_date VARCHAR2(32);
   v_start_time_hour VARCHAR2(4);
   v_start_time_min VARCHAR2(4);
   v_start_time_ampm VARCHAR2(4);
   v_start_time VARCHAR2(32);
   v_end_month VARCHAR2(4);
   v_end_month_day VARCHAR2(4);
   v_end_year VARCHAR2(4);
   v_end_date VARCHAR2(32);
   v_end_time_hour VARCHAR2(4);
   v_end_time_min VARCHAR2(4);
   v_end_time_ampm VARCHAR2(4);
   v_end_time VARCHAR2(32);
   v_startdatetime VARCHAR2(64);
   v_enddatetime VARCHAR2(64);
   v_store VARCHAR2(20);
   v_trans_count_day NUMBER(15);
   v_trans_count_total NUMBER(15);
   v_total_days NUMBER(15);
   v_total_average_trans NUMBER(18,2);
   v_average_store_trans NUMBER(18,2);
   v_last_day_num NUMBER(2);
   v_cur_day_num NUMBER(2);
   
   
   CURSOR recdet_cursor (
       cp_vtransevent CCSOWNER.RECEIPTDETAIL.REF_EVENT%TYPE, 
       cp_vstore VARCHAR2, 
       cp_vtranstype CCSOWNER.RECEIPTDETAIL.REF_TYPE%TYPE,
       cp_vsamplestart TIMESTAMP WITH TIME ZONE,
       cp_vsampleend TIMESTAMP WITH TIME ZONE) 
    IS
    select recdet.ref_store, recdet.ref_amt, sys_extract_utc(recdet.ref_time) as utc_time, recdet.ref_time
    from ccsowner.receiptdetail recdet
    where recdet.ref_event = cp_vtransevent
    and recdet.ref_store = cp_vstore
    and recdet.ref_type = cp_vtranstype
    and recdet.createdby <> 'CONV'    --Only look at organic transactions
    and sys_extract_utc(recdet.creationdate) > sys_extract_utc(cp_vsamplestart)
    and sys_extract_utc(recdet.creationdate) < sys_extract_utc(cp_vsampleend)
    order by recdet.ref_store asc, recdet.ref_time asc;
    
   TYPE recdet_tab   IS TABLE OF recdet_cursor%ROWTYPE;
   base_recdets recdet_tab;
   
   CURSOR sto_cursor IS
    select sto.storenumber
    from ccsowner.store sto
--    where sto.pawn_conversion is not null and rownum < 2
    order by sto.storenumber asc;
    
   TYPE sto_tab      IS TABLE OF sto_cursor%ROWTYPE;
   base_stores sto_tab;

BEGIN
    v_sample_size         := 100000;  -- Change this to limit the number of rows returned across the sample
    v_sample_rate_mins    := 2;       -- Rate at which to accumulate loans from the date/time span chosen in minutes
    v_trans_event         := 'New';   --Please look at the CCSOWNER.RECEIPT_REF_EVENT table for all valid transactional events.  Common ones are:
                                     --  'Extend' for extensions, 
                                     --  'Renew' for renewals, 
                                     --  'Pickup' for pickups, 
                                     --  'New' for new pawn loans, 
                                     --  'Paydown' for paydown
                                     --  'SALE' for sale
    v_trans_type          := 1;       --Leave as-is for regular pawn transactions
  
  
  -- Set start date parameters
    v_start_month         := '10';    -- Change this to reflect the starting numeric month on the calendar (Example: 10 is October)
    v_start_month_day     := '01';    -- Change this to reflect the starting numeric day in that month (NOTE: Please be cautious to not go over the number of days for that month)
    v_start_year          := '2012';  -- Change this to reflect the starting year (NOTE: There is not much data prior to March 31st of 2010 for Pawn)  
  
  -- Set start time parameters
    v_start_time_hour    := '09';    -- Change this to reflect the starting numeric 12 hour time during the day (NOTE: There is not much data before 7 AM CST)
    v_start_time_min     := '00';    -- Change this to reflect the starting numeric minute of the hour chosen
    v_start_time_ampm    := 'AM';    -- Change this to reflect the time of day (NOTE: Can only be 'AM' or 'PM')
  
  --Set end date parameters
    v_end_month          := '10';    -- Change this to reflect the ending numeric month on the calendar (Example: 10 is October)
    v_end_month_day      := '01';    -- Change this to reflect the ending numeric day in that month (NOTE: Please be cautious to not go over the number of days for that month)
    v_end_year           := '2012';  -- Change this to reflect the ending year (NOTE: There is not much data prior to March 31st of 2010 for Pawn)  

  -- Set end time parameters
    v_end_time_hour      := '03';    -- Change this to reflect the ending numeric 12 hour time during the day (NOTE: This must be after the starting time)
    v_end_time_min       := '00';    -- Change this to reflect the ending numeric minute of the hour chosen
    v_end_time_ampm      := 'PM';    -- Change this to reflect the time of day (NOTE: Can only be 'AM' or 'PM')
  
  
  
  ----------------------------------------------------------------------------------------------------------------------
  -- DO NOT CHANGE ANYTHING BELOW THIS LINE - All modifiable parameters are listed and should be set above -------------
  
  -- Init store holder
  v_store              := '00101'; -- DO NOT CHANGE THIS - ONLY FOR INITIALIZATION PURPOSES
    
  -- Generate the start and end date
  v_start_date   := lpad(v_start_month, 2, '0') || '/' || lpad(v_start_month_day, 2, '0') || '/' || v_start_year;
  v_end_date     := lpad(v_end_month, 2, '0') || '/' || lpad(v_end_month_day, 2, '0') || '/' || v_end_year;

  -- Generate the start and end time
  v_start_time   := lpad(v_start_time_hour, 2, '0') || ':' || lpad(v_start_time_min, 2, '0') || ' ' || v_start_time_ampm;
  v_end_time     := lpad(v_end_time_hour, 2, '0') || ':' || lpad(v_end_time_min, 2, '0') || ' ' || v_end_time_ampm;
  v_startdatetime := v_start_date || ' ' || v_start_time;
  v_enddatetime  := v_end_date || ' ' || v_end_time;
  v_sample_start := to_timestamp_tz(v_startdatetime, 'MM/DD/YYYY HH:MI PM');
  v_sample_end   := to_timestamp_tz(v_enddatetime,'MM/DD/YYYY HH:MI PM');
  
  -- Starting fetch of store data
  OPEN sto_cursor;
  FETCH sto_cursor BULK COLLECT INTO base_stores; 
  CLOSE sto_cursor;
    
  -- Starting data analysis and output
  dbms_output.put_line('TIME,SAMPLE_DURATION_MINUTES, NUMBER_LOANS, TOTAL_NUMBER_LOANS, LOANS_PER_SECOND, TOTAL_LOANS_PER_SECOND, STORE_NUMBER, AVG_PER_DAY');
  FOR j IN base_stores.FIRST .. base_stores.LAST
  LOOP
      -- Reset key parameters per store
      v_duration := 0;
      v_total := 0;
      v_total_seconds := 0;
      v_curmin := 0;
      v_last_sample_min := 0;
      v_isbegin := '1';
      v_issamplebegin := '1';
      v_count := 1; --Always start at 1
      
      --Init average counters
      v_trans_count_day := 1;
      v_trans_count_total := 1;
      v_total_days := 1;
      v_total_average_trans := 1;
      v_average_store_trans := 1;
      v_last_day_num := 1;
      v_cur_day_num := 1;
      
      --Set current store
      v_store := base_stores(j).storenumber;
      
      --base_recdets := recdet_tab;
      DBMS_OUTPUT.PUT_LINE('Opening cursor for store number: ' || v_store);
      OPEN recdet_cursor(v_trans_event, v_store, v_trans_type, v_sample_start, v_sample_end);
      -- Starting fetch of transaction data
      DBMS_OUTPUT.PUT_LINE('Cursor fetch starting');
      FETCH recdet_cursor BULK COLLECT INTO base_recdets LIMIT v_sample_size;
      DBMS_OUTPUT.PUT_LINE('Cursor fetch finished');
      IF recdet_cursor%NOTFOUND
      THEN
          CLOSE recdet_cursor;
          
          CONTINUE;                    
      END IF;
      CLOSE recdet_cursor;
      DBMS_OUTPUT.PUT_LINE('Cursor closed');
      -- End fetch of transaction data

      -- Loop through the entire data set returned from the cursor for this store
      DBMS_OUTPUT.PUT_LINE('Starting loop');
      
      FOR i IN base_recdets.FIRST .. base_recdets.LAST
      LOOP
        DBMS_OUTPUT.PUT_LINE('** Iteration('||i||')**');
      
        DBMS_OUTPUT.PUT_LINE('starting with base receipt details');
        IF (v_issamplebegin = '1')    
        THEN
            -- If this is the first iteration, snap the minute from the first transaction
            v_last_sample_min := to_number(lpad(to_char(base_recdets(i).utc_time, 'MI'), 2, '0'), '99');
            v_curmin := v_last_sample_min;
            v_issamplebegin := '0';   
            v_cur_day_num := to_number(lpad(to_char(base_recdets(i).utc_time, 'DD'), 2, '0'), '99');
            v_last_day_num := v_cur_day_num;  
        ELSE  
            -- Capture current transaction
            v_curmin := to_number(lpad(to_char(base_recdets(i).utc_time, 'MI'), 2, '0'), '99');    
            v_cur_day_num := to_number(lpad(to_char(base_recdets(i).utc_time, 'DD'), 2, '0'), '99');
            dbms_output.put_line('Vcurmin := ' || v_curmin || ', Vcurdaynum ' || v_cur_day_num);
                
            /*IF (v_last_day_num <> v_cur_day_num)
            THEN
                v_total_days := v_total_days + 1;    
                v_trans_count_total := v_trans_count_total + v_trans_count_day;          
                v_average_store_trans := v_trans_count_total / v_total_days;

                dbms_output.put_line('null' || ', ' || 'null' || ', ' || 'null' || ', ' || 'null' || ', ' || 'null' || ', ' || 'null' || ', ' || v_store || ', ' || v_average_store_trans);
                --Reset average counters
                v_trans_count_day := 1;
                v_last_day_num := 1;
                v_cur_day_num := 1;
            END IF;*/
                
        END IF;
            
        -- Compute duration if v_curmin is greater than last sample min
        IF (v_curmin >= v_last_sample_min)
        THEN
            v_duration := v_curmin - v_last_sample_min;
        -- Otherwise, we've crossed the hour boundary and need to compute differently
        ELSE
            v_duration := (59 - v_last_sample_min) + v_curmin;
        END IF;
            
        -- If our duration matches our sampling rate, or is greater than, print out the next stamp
        IF (v_duration = 0 OR v_duration >= v_sample_rate_mins)
        THEN
            IF v_last_sample_min <> v_curmin
            THEN
                IF v_isbegin <> '1'
                THEN 
                    v_total := v_total + v_count;  
                    v_trans_count_day := v_trans_count_day + v_count;                               
                    v_local_time := base_recdets(i).ref_time AT TIME ZONE('America/Chicago');
                    v_sample_loans_second := v_count / ((v_duration+1) * 60);
                    v_total_seconds := v_total_seconds + ((v_duration+1) * 60);
                    v_total_loans_second := v_total / (v_total_seconds+1);
                    dbms_output.put_line(v_local_time || ', ' || v_duration || ', ' || v_count || ', ' || v_total || ', ' || v_sample_loans_second || ', ' || v_total_loans_second || ', ' || v_store || ', ' || '0');
                    v_count := 1;  --Always start at one
                    v_last_sample_min := v_curmin;            
                ELSE
                    v_count := v_count + 1;
                END IF;
                v_isbegin := '0';
            ELSE
                v_count := v_count + 1;
            END IF;    
        ELSE
            v_count := v_count + 1;
        END IF;
            
      END LOOP;
      --END IF;
      TRUNCATE base_recdets; 
    END LOOP;  
  
    
END;