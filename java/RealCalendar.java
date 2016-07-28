/**
 * 
 */
package com.casham.dsos.utils;

import java.text.DateFormat;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.GregorianCalendar;
import java.util.HashMap;

import com.casham.dsos.exception.DSOSExceptionHandler;
import com.casham.dsos.exception.DSOSLogger;

/**
 * @author glepag1
 * Wrapper class for GregorianCalendar
 * Provides consistent handling of real life date values
 * Only provides this functionality for primary calendar values (e.g. day, month, and year)
 * This wrapper does not take time into account
 */
public class RealCalendar
{
    public static final boolean LOG = false;
    public static final String DEFAULT_DATE_PATTERN = "MM/dd/yyyy";
    public static final SimpleDateFormat DEFAULT_DATE_FORMATTER = 
        new SimpleDateFormat(DEFAULT_DATE_PATTERN);
    public static final String SQL_DATE_PATTERN = "yyyy-MM-dd";
    public static final SimpleDateFormat SQL_DATE_FORMATTER = 
        new SimpleDateFormat(SQL_DATE_PATTERN);
    public static final int REAL_MONTH_OFFSET = 1;
    public static final int LEAP_YEAR_DAY = 29;
    
    /**
     * Internal enumeration for
     * months of the year
     * Each month holds actual calendar
     * value.
     */
    public enum RealMonth
    {
        JANUARY(1), 
        FEBRUARY(2), 
        MARCH(3), 
        APRIL(4), 
        MAY(5), 
        JUNE(6), 
        JULY(7), 
        AUGUST(8), 
        SEPTEMBER(9),
        OCTOBER(10),
        NOVEMBER(11),
        DECEMBER(12);
        
        private int monthVal;
        private RealMonth(int val)
        {
            this.monthVal = val;
        }
        
        public int getMonthVal()
        {
            return(this.monthVal);
        }
        
        public String toString()
        {
            return(""+monthVal);
        }
    };
    public static final int MIN_DAY_OF_MONTH       = 1;
    public static final RealMonth MIN_MONTH = RealMonth.JANUARY;
    public static final int MIN_REAL_MONTH = MIN_MONTH.getMonthVal();
    public static final RealMonth MAX_MONTH = RealMonth.DECEMBER;
    public static final int MAX_REAL_MONTH = MAX_MONTH.getMonthVal();
    
    public static final int[] NON_LEAP_YEAR_MONTH_DAYS_MAX = 
    {
        //Jan, Feb, Mar
        31, 28, 31,
        //Apr, May, Jun
        30, 31, 30,
        //Jul, Aug, Sep
        31, 31, 30,
        //Oct, Nov, Dec
        31, 30, 31
    };
    
    public static final int[] LEAP_YEAR_MONTH_DAYS_MAX =
    {
        //Jan, Feb, Mar
        31, 29, 31,
        //Apr, May, Jun
        30, 31, 30,
        //Jul, Aug, Sep
        31, 31, 30,
        //Oct, Nov, Dec
        31, 30, 31
    };
    
    public static final RealMonth[] REAL_MONTHS =
    {
        RealMonth.JANUARY, RealMonth.FEBRUARY, RealMonth.MARCH,
        RealMonth.APRIL, RealMonth.MAY, RealMonth.JUNE, 
        RealMonth.JULY, RealMonth.AUGUST, RealMonth.SEPTEMBER,
        RealMonth.OCTOBER, RealMonth.NOVEMBER, RealMonth.DECEMBER
    };
    
    public static final int[] CALENDAR_MONTHS =
    {
        Calendar.JANUARY, Calendar.FEBRUARY, Calendar.MARCH,
        Calendar.APRIL, Calendar.MAY, Calendar.JUNE,
        Calendar.JULY, Calendar.AUGUST, Calendar.SEPTEMBER,
        Calendar.OCTOBER, Calendar.NOVEMBER, Calendar.DECEMBER        
    };
    
    public static final int[] ACTUAL_DAYS_OF_WEEK_NUMERIC =
    {
        1, 2, 3,
        4, 5, 6,
        7        
    };
    
    public static final String[] ACTUAL_DAYS_OF_WEEK_STRING =
    {
        "Sunday", "Monday", 
        "Tuesday", "Wednesday", 
        "Thursday", "Friday", 
        "Saturday"
    };
    
    public static final int[] CALENDAR_DAYS_OF_WEEK =
    {
        Calendar.SUNDAY, Calendar.MONDAY, 
        Calendar.TUESDAY, Calendar.WEDNESDAY, 
        Calendar.THURSDAY, Calendar.FRIDAY, 
        Calendar.SATURDAY
    };
    
    public static final DateComponent[] MONTH_DAY_YEAR =
    {
        DateComponent.MONTH, DateComponent.DAY, DateComponent.YEAR
    };
    public static final DateComponent[] YEAR_MONTH_DAY =
    {
        DateComponent.YEAR, DateComponent.MONTH, DateComponent.DAY
    };

    
    public static final int INVALID_DATE_VAL = -1;
    public static final String INVALID_DATE_STR = "<invalid date>";
    public static final java.util.Date INVALID_JUTIL_DATE = new java.util.Date(0L);
    public static final java.sql.Date INVALID_SQL_DATE = new java.sql.Date(0L);
    public enum DateComponent
    {
        YEAR, MONTH, DAY;
    };
    public static final int NUMBER_DATE_COMPONENTS = 3; 
    public static final int NUMBER_DATE_SEPARATORS = 2;
    public static final char DEFAULT_DATE_SEPARATOR = '/';
    public static final char JAVASQL_DATE_SEPARATOR = '-';
    
    
    //Internal GregorianCalendar reference
    private GregorianCalendar calendar;
    
    //Real calendar values
    private boolean originalLeapYearDay;
    private boolean originalMaxMonth;
    private boolean isLeapYear;
    private boolean isLeapYearDay;
    private RealMonth currentMonth;
    private int daysInMonth;
    private int currentDayOfWeek;
    private String currentDayOfWeekStr;
    private int currentDayOfYear;
    private HashMap<DateComponent, Integer> currentDate;
    private java.sql.Date currentSqlDate;
    private java.util.Date currentJavaDate;
    private long currentSystemTime;
    private String dateString;
    
    //Logger
    private DSOSLogger logger;
    
    //Month maps
    //Real Calendar to Gregorian (Month)
    private static HashMap<RealMonth, Integer> monthMapRealToGreg;
    //Gregorian Calendar to Real (Month)
    private static HashMap<Integer, RealMonth> monthMapGregToReal;
    
    //Day of week maps
    //Real Calendar (Named) to Gregorian   (Day of Week)
    private static HashMap<String, Integer> dayOfWeekMapRealToGreg;
    //Real Calendar (Numeric) to Gregorian (Day of Week)
    private static HashMap<Integer, Integer> dayOfWeekMapRealNumToGreg;
    //Gregorian Calendar to Real (Named)   (Day of Week)
    private static HashMap<Integer, String> dayOfWeekMapGregToReal;
    //Gregorian Calendar to Real (Numeric) (Day of Week)
    private static HashMap<Integer, Integer> dayOfWeekMapGregToRealNum;

    /**
     * Initialize static calendar data
     */
    public static void initializeCalendarData()
    {
        RealCalendar.generateDayOfWeekMaps();
        RealCalendar.generateMonthMaps();
    }
    
    static
    {
        RealCalendar.initializeCalendarData();
    }
    
    /**
     * Static method to parse a date string given a component sequence ordering map,
     * the date string itself, and the separator character
     * @param order - Date component sequence map
     * @param date  - Date string to parse
     * @param sep   - Separator character
     * @return HashMap<DateComponent, Integer> - Each DateComponent's calendar value
     */
    public static HashMap<DateComponent, Integer> parseDate(DateComponent[] order, String date, char sep)
    {
        if (StringUtils.isEmpty(date) || CollectionUtils.isEmpty(order) || order.length != NUMBER_DATE_COMPONENTS)
        {
            return(null);
        }
        DSOSLogger log = DSOSExceptionHandler.getInstance().getLogger();
        if (LOG)if (log != null)log.logInfo(RealCalendar.class, "parseDate(" + order.toString() + ", " + date + ", " + sep + ")...");
        
        //Create return object
        HashMap<DateComponent, Integer> rt = new HashMap<DateComponent, Integer>();
        
        //Get and verify indices of separator character
        int lastIdx = -1;
        int[] indices = new int[NUMBER_DATE_SEPARATORS];
        for (int j = 0; j < NUMBER_DATE_SEPARATORS; ++j)
        {
            if (lastIdx == -1)
            {
                indices[j] = date.indexOf(sep);
                if (indices[j] == -1)return(null);
                lastIdx = indices[j];
                if (LOG)if (log != null)log.logInfo(RealCalendar.class, "Found separator at " + indices[j]);
            }
            else
            {
                indices[j] = date.indexOf(sep, lastIdx + 1);
                if (indices[j] == lastIdx)return(null);
                lastIdx = indices[j];
                if (LOG)if (log != null)log.logInfo(RealCalendar.class, "Found separator at " + indices[j]);
            }
        }
        
        //Pull values from date string in the order they are specified
        for (int j = 0; j < NUMBER_DATE_COMPONENTS; ++j)
        {
            DateComponent d = order[j];
            if (d == null)
            {
                return(null);
            }
            
            int curCompBegIdx = -1;
            int curCompEndIdx = -1;
            if (j == 0)
            {
                curCompBegIdx = 0;
                curCompEndIdx = indices[0];
            }
            else if (j < NUMBER_DATE_SEPARATORS)
            {
                curCompBegIdx = indices[j-1] + 1;
                curCompEndIdx = indices[j];
            }
            else
            {
                curCompBegIdx = indices[j-1] + 1;
                curCompEndIdx = date.length();
            }
            if (LOG)if (log != null)log.logInfo(RealCalendar.class, "DateComponent("+d.name()+") begIdx = " + curCompBegIdx + ", endIdx = " + curCompEndIdx);
            
            if (curCompBegIdx == -1 || curCompEndIdx == -1)
            {
                return(null);
            }
            
            String compVal = date.substring(curCompBegIdx, curCompEndIdx);
            if (StringUtils.isEmpty(compVal))
            {
                return(null);
            }
            if (LOG)if (log != null)log.logInfo(RealCalendar.class, "DateComponent("+d.name()+") value = " + compVal);
            rt.put(d, Integer.parseInt(compVal));
        }
        
        if (CollectionUtils.isEmpty(rt))
        {
            rt = null;
        }
        
        return(rt);
        
    }
    
    /**
     * Given a calendar month and year, returns the maximum number of days
     * @param realMonthVal - Calendar month value
     * @param realYearVal  - Calendar year value
     * @return - Maximum number of days for that month
     */
    public static int getMaxDaysForMonthYear(int realMonthVal, int realYearVal)
    {
        int rt = INVALID_DATE_VAL;
        if (realMonthVal >= MIN_REAL_MONTH &&
            realMonthVal <= MAX_REAL_MONTH &&
            realYearVal > 0)
        {
            RealMonth realMonth = REAL_MONTHS[realMonthVal-REAL_MONTH_OFFSET];
            rt = RealCalendar.getMaxDaysForMonthYear(realMonth, realYearVal);
        }
        return(rt);
    }
    
    /**
     * Given a RealMonth object and a year, returns the maximum number of days
     * @param realMonth   - Real month enumerated value
     * @param realYearVal - Calendar year value
     * @return - Maximum number of days for that month
     */
    public static int getMaxDaysForMonthYear(RealMonth realMonth, int realYearVal)
    {
        int rt = INVALID_DATE_VAL;
        if (realMonth != null && realYearVal > 0)
        {
            boolean realYearIsLeap = new GregorianCalendar().isLeapYear(realYearVal);
            if (realYearIsLeap)
            {
                rt = RealCalendar.LEAP_YEAR_MONTH_DAYS_MAX[realMonth.getMonthVal()-RealCalendar.REAL_MONTH_OFFSET];
            }
            else
            {
                rt = RealCalendar.NON_LEAP_YEAR_MONTH_DAYS_MAX[realMonth.getMonthVal()-RealCalendar.REAL_MONTH_OFFSET];
            }
        }
        return(rt);
    }
    
    /**
     * Returns whether or not the specified day is a leap year day
     * @param realDayVal  - Calendar day of month
     * @param realMonth   - Real month enumerated value
     * @param realYearVal - Calendar year value
     * @return - 
     */
    public static boolean isLeapYearDay(int realDayVal, RealMonth realMonth, int realYearVal)
    {
        boolean rt = false;
        if (realMonth != null && realDayVal >= RealCalendar.MIN_DAY_OF_MONTH && realYearVal > 0)
        {
            if (realMonth == RealMonth.FEBRUARY)
            {
                boolean isLeapYear = new GregorianCalendar().isLeapYear(realYearVal);
                if (isLeapYear && realDayVal == RealCalendar.LEAP_YEAR_MONTH_DAYS_MAX[realMonth.getMonthVal()-RealCalendar.REAL_MONTH_OFFSET])
                {
                    rt = true;
                }
            }
        }
        return(rt);
    }
    

    /**
     * 
     */
    private static void generateMonthMaps()
    {
        RealCalendar.monthMapRealToGreg = new HashMap<RealMonth, Integer>();
        RealCalendar.monthMapGregToReal = new HashMap<Integer, RealMonth>();
        for (int j = 0; j < REAL_MONTHS.length; ++j)
        {
            RealMonth act = REAL_MONTHS[j];
            Integer cal = new Integer(CALENDAR_MONTHS[j]);
            RealCalendar.monthMapRealToGreg.put(act, cal);
            RealCalendar.monthMapGregToReal.put(cal, act);
        }
    }
    
    /**
     * 
     */
    private static void generateDayOfWeekMaps()
    {
        RealCalendar.dayOfWeekMapRealToGreg = new HashMap<String, Integer>();
        RealCalendar.dayOfWeekMapRealNumToGreg = new HashMap<Integer, Integer>();
        RealCalendar.dayOfWeekMapGregToReal = new HashMap<Integer, String>();
        RealCalendar.dayOfWeekMapGregToRealNum = new HashMap<Integer, Integer>();
        
        for (int j = 0; j < ACTUAL_DAYS_OF_WEEK_NUMERIC.length; ++j)
        {
            Integer wkRealNum = new Integer(ACTUAL_DAYS_OF_WEEK_NUMERIC[j]);
            String  wkReal    = ACTUAL_DAYS_OF_WEEK_STRING[j]; 
            Integer wkGreg    = new Integer(CALENDAR_DAYS_OF_WEEK[j]);
            
            RealCalendar.dayOfWeekMapRealToGreg.put(wkReal, wkGreg);
            RealCalendar.dayOfWeekMapRealNumToGreg.put(wkRealNum, wkGreg);
            RealCalendar.dayOfWeekMapGregToReal.put(wkGreg, wkReal);
            RealCalendar.dayOfWeekMapGregToRealNum.put(wkGreg, wkRealNum);
        }        
    }
    
    /**
     * Update all other internal values based on the internal calendar
     */
    private void updateInternalData(boolean firstTime)
    {
        if (LOG)this.logMsg("updateInternalData()...");
        //Formulate internal data
        int dayOfMonthFromCal = this.calendar.get(GregorianCalendar.DAY_OF_MONTH);
        int yearValFromCal    = this.calendar.get(GregorianCalendar.YEAR);
        int monthValFromCal   = this.calendar.get(GregorianCalendar.MONTH);
        int dayOfWeekFromCal  = this.calendar.get(GregorianCalendar.DAY_OF_WEEK);
        if (LOG)this.logMsg("- Day of Month From Cal = " + dayOfMonthFromCal);
        if (LOG)this.logMsg("- Year Value From Cal   = " + yearValFromCal);
        if (LOG)this.logMsg("- Month Value From Cal  = " + monthValFromCal);
        if (LOG)this.logMsg("- Day Of Week From Cal  = " + dayOfWeekFromCal);
        
        //Set current day of year
        this.currentDayOfYear = this.calendar.get(GregorianCalendar.DAY_OF_YEAR);
        if (LOG)this.logMsg("- Current Day Of Year   = " + this.currentDayOfYear);
        
        //Get real values from Gregorian values - translate only month and day of week(num and str)
        this.currentDayOfWeekStr = RealCalendar.dayOfWeekMapGregToReal.get(dayOfWeekFromCal);
        this.currentDayOfWeek    = RealCalendar.dayOfWeekMapGregToRealNum.get(dayOfWeekFromCal);        
        this.currentMonth        = RealCalendar.monthMapGregToReal.get(monthValFromCal);
        if (this.currentMonth == null)
        {
            throw new RuntimeException("RealCalendar::updateInternalData - Cannot retreive RealMonth object from gregorian month value");
        }
        if (LOG)this.logMsg("- Real Day Of Week      = " + this.currentDayOfWeekStr);
        if (LOG)this.logMsg("- Real Month Value      = " + this.currentMonth.getMonthVal()); 
        
        //Formulate date string
        this.dateString = "" + this.currentMonth + DEFAULT_DATE_SEPARATOR + dayOfMonthFromCal + DEFAULT_DATE_SEPARATOR + yearValFromCal;
        if (LOG)this.logMsg("- Real Date String      = " + this.dateString);
        
        //Store values in current date map
        if (CollectionUtils.isEmpty(this.currentDate))
        {
            this.currentDate = new HashMap<DateComponent, Integer>();
        }
        this.currentDate.put(DateComponent.DAY, dayOfMonthFromCal);
        this.currentDate.put(DateComponent.MONTH, this.currentMonth.getMonthVal());
        this.currentDate.put(DateComponent.YEAR, yearValFromCal);
        if (LOG)this.logMsg("- Real Current Date     = " + CollectionUtils.convertToString(this.currentDate));
        
        //Set leap year values
        this.isLeapYear = this.calendar.isLeapYear(yearValFromCal);
        if (this.isLeapYear)
        {
            this.daysInMonth = LEAP_YEAR_MONTH_DAYS_MAX[this.currentMonth.getMonthVal()-REAL_MONTH_OFFSET];
        }
        else
        {
            this.daysInMonth = NON_LEAP_YEAR_MONTH_DAYS_MAX[this.currentMonth.getMonthVal()-REAL_MONTH_OFFSET];
        }
        if (LOG)this.logMsg("- Number Days In Month  = " + this.daysInMonth);
        this.isLeapYearDay = false;
        
        //Set original leap year day flag
        if (this.isLeapYear)
        {
            if (this.currentMonth == RealMonth.FEBRUARY)
            {
                if (dayOfMonthFromCal == this.daysInMonth)
                {
                    this.isLeapYearDay = true;
                    if (firstTime)
                    {
                        this.originalLeapYearDay = true;
                    }
                }
            }
        }
        
        //Set max of month flag
        if (!this.originalLeapYearDay && 
            dayOfMonthFromCal == this.daysInMonth)
        {
            if (firstTime)
            {
                this.originalMaxMonth = true;
            }
        }
        
        if (LOG)this.logMsg("- Is Leap Year          = " + this.isLeapYear);
        if (LOG)this.logMsg("- Is Leap Year Day      = " + this.isLeapYearDay);
        if (LOG)this.logMsg("- Original Leap Year    = " + this.originalLeapYearDay);
        if (LOG)this.logMsg("- Original Max Of Month = " + this.originalMaxMonth);
        
        //Set Java-centric Date storage objects
        try
        {
            SimpleDateFormat sdfJavaUtil = new SimpleDateFormat(DEFAULT_DATE_PATTERN);
            this.currentJavaDate = sdfJavaUtil.parse(this.dateString);
            if (LOG)this.logMsg("- Real JavaUtil Date    = " + this.currentJavaDate.toString());
            this.currentSystemTime = this.currentJavaDate.getTime();
            if (LOG)this.logMsg("- Real SystemTime       = " + this.currentSystemTime);
            this.currentSqlDate = new java.sql.Date(this.currentSystemTime);
            if (LOG)this.logMsg("- Real JavaSQL Date     = " + this.currentSqlDate);
        }
        catch (ParseException e)
        {
            throw new RuntimeException("RealCalendar::initializeCalendarData(String, HashMap, char) - Cannot compute java.util.Date");
        }        
    }
    
    public RealCalendar getStartOfMonth()
    {
        RealCalendar rt = null;
        if (this.isValid())
        {
            int curMonthVal = this.currentDate.get(DateComponent.MONTH);
            int curYearVal  = this.currentDate.get(DateComponent.YEAR);
            int minDayVal = MIN_DAY_OF_MONTH;
            rt = new RealCalendar(curMonthVal, minDayVal, curYearVal);            
        }
        return(rt);
    }
    
    public RealCalendar getEndOfMonth()
    {
        RealCalendar rt = null;
        if (this.isValid())
        {
            int curMonthVal = this.currentDate.get(DateComponent.MONTH);
            int curYearVal  = this.currentDate.get(DateComponent.YEAR);
            int maxDayVal   = RealCalendar.getMaxDaysForMonthYear(curMonthVal, curYearVal);
            rt = new RealCalendar(curMonthVal, maxDayVal, curYearVal);
        }
        return(rt);
    }
    
    private void logMsg(String msg)
    {
        if (!LOG)return;
        if (this.logger == null)
        {
            this.logger = DSOSExceptionHandler.getInstance().getLogger();
            if (this.logger == null)
            {
                this.logger = new DSOSLogger(DSOSLogger.LogThreshold.LOGDEBUG, System.out);
            }
        }        
        this.logger.logInfo(this, msg);
    }
    
    /**
     * 
     */
    private void initializeData()
    {
        this.logger = null;
        this.calendar = null;
        this.originalLeapYearDay = false;
        this.originalMaxMonth = false;
    }
    
    private void initializeCalendarData(GregorianCalendar cal, boolean firstTime)
    {
        if (cal == null)
        {
            throw new RuntimeException("RealCalendar::initializeCalendarData(GregorianCalendar) - Calendar is null");
        }
        this.calendar = (GregorianCalendar)cal.clone();
        if (LOG)this.logMsg("Gregorian calendar created = " + this.calendar.toString());
        this.updateInternalData(firstTime);
    }
    
    /**
     * Take current incoming data and create an internal calendar
     * @param cal
     * @param comp
     * @param sep
     */
    private void initializeCalendarData(String cal, DateComponent[] comp, char sep, boolean firstTime)
    {
        if (StringUtils.isEmpty(cal) || CollectionUtils.isEmpty(comp))
        {            
            throw new RuntimeException("RealCalendar::initializeCalendarData(String, HashMap, char) - Parameters are invalid");
        }
        
        if (LOG)this.logMsg("initializeCalendarData(" + cal + ", " + comp.toString() + ", '" + sep + "')...");
        
        HashMap<DateComponent, Integer> values = RealCalendar.parseDate(comp, cal, sep);
        if (CollectionUtils.isEmpty(values))
        {
            throw new RuntimeException("RealCalendar::initializeCalendarData(String, HashMap, char) - Cannot retrieve date values");            
        }
        
        if (LOG)this.logMsg("initializeCalendarData has correct values");
        
        //Set internal date
        this.currentDate = values;
        
        //Get date components to create calendar
        int yearVal = this.currentDate.get(DateComponent.YEAR);
        int dayVal  = this.currentDate.get(DateComponent.DAY);
        int monVal  = this.currentDate.get(DateComponent.MONTH);
        
        if (LOG)this.logMsg("Calendar data (Y,M,D) = (" + yearVal + "," + monVal + "," + dayVal + ')');
        
        //Map month value to gregorian month value
        Integer gregMonVal = RealCalendar.monthMapRealToGreg.get(REAL_MONTHS[monVal-REAL_MONTH_OFFSET]);
        if (gregMonVal == null)
        {
            throw new RuntimeException("RealCalendar::initializeCalendarData - Cannot retreive gregorian calendar value");
        }
        if (LOG)this.logMsg("Translated month value of " + monVal + " to Gregorian is = " + gregMonVal.intValue());
        
        //Create calendar
        this.calendar = new GregorianCalendar(yearVal, gregMonVal.intValue(), dayVal);
        if (LOG)this.logMsg("Gregorian calendar created = " + this.calendar.toString());
        
        //Update internal data based on the calendar
        this.updateInternalData(firstTime);
    }
    
    /**
     * 
     */
    public RealCalendar()
    {
        this.initializeData();
        //Initialize calendar based on current system time stamp
        this.initializeCalendarData(new GregorianCalendar(), true);
    }
    
    public RealCalendar(RealCalendar rCal)
    {
        this.initializeData();
        if (rCal != null && rCal.isValid())
        {
            //Default pattern is month/day/year
            this.initializeCalendarData(rCal.toString(), MONTH_DAY_YEAR, DEFAULT_DATE_SEPARATOR, true);        
        }
        else
        {
            throw new RuntimeException("RealCalendar(): RealCalendar parameter is invalid");
        }
    }
    
    /**
     * @param cal
     */
    public RealCalendar(GregorianCalendar cal)
    {
        if (cal == null)
        {
            throw new RuntimeException("RealCalendar(): GregorianCalendar parameter is invalid");
        }
        this.initializeData();
        if (LOG)this.logMsg("RealCalendar(GregorianCalendar) - calendar = " + cal.toString());
        //Initialize calendar based on GregorianCalendar instance
        this.initializeCalendarData(cal, true);
    }
    
    /**
     * @param systemTime
     */
    public RealCalendar(long systemTime)
    {
        if (systemTime < 0L)
        {
            throw new RuntimeException("RealCalendar(): System time cannot be less than 0");
        }
        this.initializeData();
        if (LOG)this.logMsg("RealCalendar(long) - system time = " + systemTime);
        
        //Create java.util.Date based on system time
        java.util.Date date = new java.util.Date(systemTime);
        SimpleDateFormat sdf = new SimpleDateFormat(DEFAULT_DATE_PATTERN);
        String dateStr = sdf.format(date);
        if (LOG)this.logMsg("- Date string created from system time = " + dateStr);
        
        //Default pattern is month/day/year
        this.initializeCalendarData(dateStr, MONTH_DAY_YEAR, DEFAULT_DATE_SEPARATOR, true);        
    }
    
    /**
     * @param date
     */
    public RealCalendar(java.util.Date date)
    {
        if (date == null)
        {
            throw new RuntimeException("RealCalendar(): java.util.Date parameter is invalid");
        }
        this.initializeData();
        //Initialize calendar based on java.util.Date instance
        SimpleDateFormat sdf = new SimpleDateFormat(DEFAULT_DATE_PATTERN);
        String dateStr = sdf.format(date);
        if (LOG)this.logMsg("RealCalendar(java.util.Date) - dateStr created = " + dateStr);
        
        //Default pattern is month/day/year
        this.initializeCalendarData(dateStr, MONTH_DAY_YEAR, DEFAULT_DATE_SEPARATOR, true);
    }
    
    /**
     * @param date
     */
    public RealCalendar(java.sql.Date date)
    {
        if (date == null)
        {
            throw new RuntimeException("RealCalendar(): java.sql.Date parameter is invalid");
        }
        this.initializeData();
        //Initialize calendar based on java.sql.Date instance
        String dateStr = date.toString();
        if (LOG)this.logMsg("RealCalendar(java.sql.Date) - dateStr = " + dateStr);
        
        //Java.sql.Date pattern is year-month-day
        this.initializeCalendarData(dateStr, YEAR_MONTH_DAY, JAVASQL_DATE_SEPARATOR, true);
        
    }
    
    /**
     * @param dateStr
     * @param format
     */
    public RealCalendar(String dateStr, DateFormat format)
    {
        if (StringUtils.isEmpty(dateStr) || format == null)
        {
            throw new RuntimeException("RealCalendar(): String dateStr and/or DateFormat format objects are invalid");
        }
        this.initializeData();
        //Initialize calendar based on a string with specified formatter object
        try
        {
            java.util.Date date = format.parse(dateStr);
            SimpleDateFormat sdf = new SimpleDateFormat(DEFAULT_DATE_PATTERN);
            String formattedStr = sdf.format(date);
            if (LOG)this.logMsg("RealCalendar(String,DateFormat) - formattedstr = " + formattedStr);

            //Default pattern is month/day/year
            this.initializeCalendarData(formattedStr, MONTH_DAY_YEAR, DEFAULT_DATE_SEPARATOR, true);
        }
        catch (ParseException e)
        {
            throw new RuntimeException("RealCalendar(String, DateFormat): Parse exception - " + e.getMessage());
        }
    }
    
    /**
     * @param dateStr
     * @param dateFormatStr - Format string that must comply with SimpleDateFormat rules
     */
    public RealCalendar(String dateStr, String dateFormatStr)
    {
        if (StringUtils.isEmpty(dateStr) || StringUtils.isEmpty(dateFormatStr))
            throw new RuntimeException("RealCalendar(): String dateStr and/or String dateFormatStr objects are invalid");
        this.initializeData();
        //Initialize calendar based on a string with specified format string
        //which maps to SimpleDateFormat class specified format characters
        try
        {
            if (LOG)this.logMsg("RealCalendar(String, String) - Inputs = " + dateStr + ", " + dateFormatStr);
            SimpleDateFormat sdfIn = new SimpleDateFormat(dateFormatStr);
            java.util.Date date = sdfIn.parse(dateStr);
            if (LOG)this.logMsg("- java.util.Date intermediate = " + date.toString());
            SimpleDateFormat sdfTrans = new SimpleDateFormat(DEFAULT_DATE_PATTERN);
            String formattedStr = sdfTrans.format(date);
            if (LOG)this.logMsg("- transformed string = " + formattedStr);
            
            //Default pattern is month/day/year
            this.initializeCalendarData(formattedStr, MONTH_DAY_YEAR, DEFAULT_DATE_SEPARATOR, true);
        }
        catch (ParseException e)
        {
            throw new RuntimeException("RealCalendar(String, String): Parse exception - " + e.getMessage());
        }
    }
    
    /**
     * @param realMonthVal
     * @param realDayOfMonthVal
     * @param realYearVal
     */
    public RealCalendar(int realMonthVal, int realDayOfMonthVal, int realYearVal) 
    {
        if (realYearVal < 0)
        {
            throw new RuntimeException("RealCalendar(): Real year value is invalid");
        }

        if (realMonthVal < MIN_REAL_MONTH || 
            realMonthVal > MAX_REAL_MONTH)
        {
            throw new RuntimeException("RealCalendar(): Real month value is invalid");
        }

        RealMonth rMonth = REAL_MONTHS[realMonthVal-REAL_MONTH_OFFSET];
        boolean isLeapYear = new GregorianCalendar().isLeapYear(realYearVal);
        int maxDays = (isLeapYear) ? 
                LEAP_YEAR_MONTH_DAYS_MAX[rMonth.getMonthVal()-REAL_MONTH_OFFSET] :
                NON_LEAP_YEAR_MONTH_DAYS_MAX[rMonth.getMonthVal()-REAL_MONTH_OFFSET];
        if (realDayOfMonthVal < MIN_DAY_OF_MONTH || realDayOfMonthVal > maxDays)
        {
            throw new RuntimeException("RealCalendar(): Real day of month value is invalid");
        }
        
        this.initializeData();
        if (LOG)this.logMsg("RealCalendar(int,int,int) - Month = " + realMonthVal + ", DayOfMonth = " + realDayOfMonthVal + ", Year = " + realYearVal);
        //Initialize calendar based on real month, real day, and real year value
        StringBuffer dateStringBuf = new StringBuffer();
        dateStringBuf.append(realMonthVal);
        dateStringBuf.append(DEFAULT_DATE_SEPARATOR);
        dateStringBuf.append(realDayOfMonthVal);
        dateStringBuf.append(DEFAULT_DATE_SEPARATOR);
        dateStringBuf.append(realYearVal);
        String dateStr = dateStringBuf.toString();
        if (LOG)this.logMsg("- computed dateStr = " + dateStr);
        
        //Default pattern is month/day/year
        this.initializeCalendarData(dateStr, MONTH_DAY_YEAR, DEFAULT_DATE_SEPARATOR, true);
    }
    
    public RealCalendar(java.sql.Timestamp t)
    {
        if (t == null)
        {
            throw new RuntimeException("RealCalendar(): Timestamp is invalid");
        }
        
        this.initializeData();
        String timeStampStr = t.toString();
        String mdyOfTimeStampStr = timeStampStr.substring(0, 10);
        
        //Initialize calendar based on java.sql.Date instance
        if (LOG)this.logMsg("RealCalendar(java.sql.Timestamp) - mdyStr = " + mdyOfTimeStampStr);
        
        //Java.sql.Timestamp substring pattern is year-month-day
        this.initializeCalendarData(mdyOfTimeStampStr, YEAR_MONTH_DAY, JAVASQL_DATE_SEPARATOR, true);        
    }
    
    /**
     * @param dC
     * @param moveForward
     * @return
     */
    private boolean changeDateComponent(DateComponent dC, boolean moveForward)
    {
        if (dC == null)return(false);
        boolean rt = false;
        if (!this.isValid())return(false);
        int changeAmt = (moveForward) ? 1 : -1;
        
        if (LOG)this.logMsg("Changing date component " + dC + " by " + changeAmt);
        int newYear = INVALID_DATE_VAL;
        int newMonth = INVALID_DATE_VAL;
        int newDay = INVALID_DATE_VAL;
        int curYear = this.currentDate.get(DateComponent.YEAR);
        int curDay = this.currentDate.get(DateComponent.DAY);
        int curMonth = this.currentDate.get(DateComponent.MONTH);
        if (LOG)this.logMsg("Date prior to value change = " + this.toString());
        int maxDaysNewMonth = INVALID_DATE_VAL;
        switch(dC)
        {
        case YEAR:
            newYear = curYear + changeAmt;
            newMonth = curMonth;
            newDay = curDay;
            maxDaysNewMonth = RealCalendar.getMaxDaysForMonthYear(newMonth, newYear);
            if (newDay > maxDaysNewMonth)
            {
                newDay = maxDaysNewMonth;
            }
            else if (this.originalLeapYearDay)
            {
                if (RealCalendar.isLeapYearDay(LEAP_YEAR_DAY, RealMonth.FEBRUARY, newYear))
                {
                    newDay = LEAP_YEAR_DAY;
                }
            }
            else if (this.originalMaxMonth)
            {
                newDay = maxDaysNewMonth;
            }
            
            if (LOG)this.logMsg("-|------------------------------------");
            if (LOG)this.logMsg("-| Changed YEAR |---------------------");
            if (LOG)this.logMsg("-| New year computed  = " + newYear);
            if (LOG)this.logMsg("-| New month computed = " + newMonth);
            if (LOG)this.logMsg("-| New day computed   = " + newDay);
            if (LOG)this.logMsg("-|------------------------------------");
            break;
        case MONTH:
            int tmpNewMonth = curMonth + changeAmt;
            if (tmpNewMonth < MIN_REAL_MONTH)
            {
                newYear = curYear - 1;
                newMonth = MAX_REAL_MONTH;
            }
            else if (tmpNewMonth > MAX_REAL_MONTH)
            {
                newYear = curYear + 1;
                newMonth = MIN_REAL_MONTH;
            }
            else
            {
                newYear = curYear;
                newMonth = tmpNewMonth;
            }
            newDay = curDay;
            maxDaysNewMonth = RealCalendar.getMaxDaysForMonthYear(newMonth, newYear);
            if (newDay > maxDaysNewMonth)
            {
                newDay = maxDaysNewMonth;
            }
            else if (this.originalLeapYearDay)
            {
                if (RealCalendar.isLeapYearDay(LEAP_YEAR_DAY, RealMonth.FEBRUARY, newYear))
                {
                    newDay = LEAP_YEAR_DAY;
                }
            }
            else if (this.originalMaxMonth)
            {
                newDay = maxDaysNewMonth;
            }
            
            if (LOG)this.logMsg("-|------------------------------------");
            if (LOG)this.logMsg("-| Changed MONTH |--------------------");
            if (LOG)this.logMsg("-| New year computed  = " + newYear);
            if (LOG)this.logMsg("-| New month computed = " + newMonth);
            if (LOG)this.logMsg("-| New day computed   = " + newDay);
            if (LOG)this.logMsg("-|------------------------------------");
            break;
        case DAY:
            int tmpNewDay = curDay + changeAmt;
            if (tmpNewDay < MIN_DAY_OF_MONTH)
            {
                //Check for month/year under flow
                newMonth = curMonth - 1;
                if (newMonth < MIN_REAL_MONTH)
                {
                    newYear = curYear - 1;
                    newMonth = MAX_REAL_MONTH;
                }
                else
                {
                    newYear = curYear;
                }
                //Set new day to max days for new month/year combo
                newDay = RealCalendar.getMaxDaysForMonthYear(newMonth, newYear);                
            }
            else if (tmpNewDay > RealCalendar.getMaxDaysForMonthYear(curMonth, curYear))
            {
                //Check for month/year over flow
                newMonth = curMonth + 1;
                if (newMonth > MAX_REAL_MONTH)
                {
                    newYear = curYear + 1;
                    newMonth = MIN_REAL_MONTH;
                }
                else
                {
                    newYear = curYear;
                }
                //Set new day to beginning day for new month/year combo
                newDay = MIN_DAY_OF_MONTH;
                
            }
            else
            {
                newYear = curYear;
                newMonth = curMonth;
                newDay = tmpNewDay;
            }
            if (LOG)this.logMsg("-|------------------------------------");
            if (LOG)this.logMsg("-| Changed DAY |----------------------");
            if (LOG)this.logMsg("-| New year computed  = " + newYear);
            if (LOG)this.logMsg("-| New month computed = " + newMonth);
            if (LOG)this.logMsg("-| New day computed   = " + newDay);
            if (LOG)this.logMsg("-|------------------------------------");
            break;
        }
        
        if (newYear != INVALID_DATE_VAL &&
            newMonth != INVALID_DATE_VAL &&
            newDay != INVALID_DATE_VAL)
        {
            String dateStr = "" + newMonth + DEFAULT_DATE_SEPARATOR + newDay + DEFAULT_DATE_SEPARATOR + newYear; 
            if (LOG)this.logMsg("Date values are valid (M/D/Y) = " + dateStr);
            
            //Default pattern is month/day/year
            this.initializeCalendarData(dateStr, MONTH_DAY_YEAR, DEFAULT_DATE_SEPARATOR, false);
            rt = true;
        }
        
        return(rt);        
    }
    
    /**
     * Moves the calendar forward by one unit of the specified date component
     * @param dC - Date component to increment
     * @throws RuntimeException
     */
    public void increment(DateComponent dC)
    {
        boolean rt = this.changeDateComponent(dC, true);
        
        if (rt == false)
        {
            throw new RuntimeException("Could not increment " + dC.name() + " component of RealCalendar object - currently set to " + this.toString());
        }
    }
    
    /**
     * Moves the calendar forward by a specified amount of 
     * the specified date component
     * Internally just loops one unit at a time
     * Convenience method
     * @param dC  - Date component to increment
     * @param amt - Amount to increment
     * @throws RuntimeException
     */
    public void increment(DateComponent dC, int amt)
    {
        boolean rt = false;
        
        if (amt >= 1)
        {
            for (int j = 0; j < amt; ++j)
            {
                rt = this.changeDateComponent(dC, true);
                if (!rt)break;
            }
        }
            
        
        if (rt == false)
        {
            throw new RuntimeException("Could not increment " + dC.name() + " component of RealCalendar object - currently set to " + this.toString());
        }
    }

    /**
     * Moves the calendar backward by one unit of the specified date component 
     * @param dC - Date component to decrement
     * @throws RuntimeException
     */
    public void decrement(DateComponent dC)
    {
        boolean rt = this.changeDateComponent(dC, false);
        
        if (rt == false)
        {
            throw new RuntimeException("Could not decrement " + dC.name() + " component of RealCalendar object - currently set to " + this.toString());
        }
    }
    
    /**
     * Moves the calendar backward by a specified amount of 
     * the specified date component
     * Internally just loops one unit at a time
     * Convenience method
     * @param dC  - Date component to decrement
     * @param amt - Amount to decrement
     * @throws RuntimeException
     */
    public void decrement(DateComponent dC, int amt)
    {
        boolean rt = false;
        
        if (amt >= 1)
        {
            for (int j = 0; j < amt; ++j)
            {
                rt = this.changeDateComponent(dC, false);
                if (!rt)break;
            }
        }
            
        if (rt == false)
        {
            throw new RuntimeException("Could not decrement " + dC.name() + " component of RealCalendar object - currently set to " + this.toString());
        }
    }
    
    //Accessors
    public final GregorianCalendar getCalendar()
    {
        return(this.calendar);
    }
    
    public int getComponent(DateComponent dC)
    {
        if (dC == null || CollectionUtils.isEmpty(this.currentDate))return(INVALID_DATE_VAL);
        Integer i = this.currentDate.get(dC);
        if (i == null)return(INVALID_DATE_VAL);
        return(i.intValue());
    }
    
    public int getRealDayOfWeekNumeric()
    {
        if (this.isValid())return(this.currentDayOfWeek);
        return(INVALID_DATE_VAL);
    }
    
    public String getDayOfWeekString()
    {
        if (this.isValid())return(this.currentDayOfWeekStr);
        return(INVALID_DATE_STR);
    }
    
    public int getDateYear()
    {
        if (this.isValid())return(this.currentDate.get(DateComponent.YEAR));
        return(INVALID_DATE_VAL);
    }
    
    public int getDateDay()
    {
        if (this.isValid())return(this.currentDate.get(DateComponent.DAY));
        return(INVALID_DATE_VAL);
    }
    
    public int getDateMonth()
    {
        if (this.isValid())return(this.currentDate.get(DateComponent.MONTH));
        return(INVALID_DATE_VAL);
    }
    
    public int getDayOfYear()
    {
        if (this.isValid())return(this.currentDayOfYear);
        return(INVALID_DATE_VAL);
    }
    
    public final java.util.Date getJavaUtilDate()
    {
        if (this.isValid())return(this.currentJavaDate);
        return(INVALID_JUTIL_DATE);
    }
    
    public final java.sql.Date getJavaSqlDate()
    {
        if (this.isValid())return(this.currentSqlDate);
        return(INVALID_SQL_DATE);
    }
    
    public long getSystemTimeDate()
    {
        if (this.isValid())return(this.currentSystemTime);
        return(INVALID_DATE_VAL);
    }
    
    public RealMonth getRealMonth()
    {
        return(this.currentMonth);        
    }
    
    public String toString()
    {
        if (this.isValid())return(this.dateString);
        return(INVALID_DATE_STR);
    }
    
    public String toFormatString(DateFormat fmt)
    {
        if (fmt != null && isValid())
        {
            SimpleDateFormat sdf = new SimpleDateFormat(DEFAULT_DATE_PATTERN);
            try
            {
                java.util.Date d = sdf.parse(this.dateString);
                String fmtD = fmt.format(d);
                if (StringUtils.isNotEmpty(fmtD))
                {
                    return(fmtD);
                }                
            }
            catch (ParseException e)
            {
            }
        }
        return(INVALID_DATE_STR);
    }
    
    /**
     * @return
     */
    public boolean isValid()
    {
        if (CollectionUtils.isEmpty(this.currentDate))return(false);
        Integer y = this.currentDate.get(DateComponent.YEAR);
        Integer m = this.currentDate.get(DateComponent.MONTH);
        Integer d = this.currentDate.get(DateComponent.DAY);
        if (y == null || m == null || d == null)return(false);
        if (y.intValue() == INVALID_DATE_VAL || 
            m.intValue() == INVALID_DATE_VAL || 
            d.intValue() == INVALID_DATE_VAL)
        {
            return(false);
        }
        return(true);
    }

    /* (non-Javadoc)
     * @see java.lang.Object#equals(java.lang.Object)
     */
    public boolean equals(Object c)
    {
        if (c == null || !(c instanceof RealCalendar))return(false);
        RealCalendar cal = (RealCalendar)c;
        if (!this.isValid() || !cal.isValid())return(false);
        
        int thisCalDay  = this.currentDate.get(DateComponent.DAY);
        int thisCalMon  = this.currentDate.get(DateComponent.MONTH);
        int thisCalYear = this.currentDate.get(DateComponent.YEAR);
        
        int inCalDay  = cal.getDateDay();
        int inCalMon  = cal.getDateMonth();
        int inCalYear = cal.getDateYear();
        
        if (thisCalDay == inCalDay &&
            thisCalMon == inCalMon &&
            thisCalYear == inCalYear)
        {
            return(true);
        }        
        return(false);
    }
    
    /**
     * @param args
     */
    public static void main(String[] args)
    {
        RealCalendar rcDefault = new RealCalendar();
        RealCalendar rcGregCal = new RealCalendar(new GregorianCalendar(2007, Calendar.DECEMBER, 31));
        RealCalendar rcInt3Cal = new RealCalendar(2, 29, 2008);
        long curSysTime = System.currentTimeMillis();
        RealCalendar rcSysTCal = new RealCalendar(curSysTime);
        RealCalendar rcJUDtCal = new RealCalendar(new java.util.Date(curSysTime));
        RealCalendar rcJSDtCal = new RealCalendar(new java.sql.Date(curSysTime));
        RealCalendar rcDFmtCal = new RealCalendar("07/31/2008", new SimpleDateFormat(RealCalendar.DEFAULT_DATE_PATTERN));
        RealCalendar rcDSqlCal = new RealCalendar("2008-08-05", RealCalendar.SQL_DATE_PATTERN);
        ArrayList<RealCalendar> calendars = new ArrayList<RealCalendar>();
        //Add all calendars to streamline these add tests
        calendars.add(rcDefault);
        calendars.add(rcGregCal);
        calendars.add(rcInt3Cal);
        calendars.add(rcSysTCal);
        calendars.add(rcJUDtCal);
        calendars.add(rcJSDtCal);
        calendars.add(rcDFmtCal);
        calendars.add(rcDSqlCal);
        System.out.println("|-------------------------------------------------------------|");
        System.out.println("|UsedSysTime                         = " + curSysTime);
        System.out.println("|-------------------------------------------------------------|");
        System.out.println("|[0]RealCalendar()                   = " + rcDefault);
        System.out.println("|[1]RealCalendar(GCal(12-31-2007))   = " + rcGregCal);
        System.out.println("|[2]RealCalendar(2008,1,1)           = " + rcInt3Cal);
        System.out.println("|[3]RealCalendar(UsedSysTime)        = " + rcSysTCal);
        System.out.println("|[4]RealCalendar(JUtil(UsedSysTime)) = " + rcJUDtCal);
        System.out.println("|[5]RealCalendar(JSQL(UsedSysTime))  = " + rcJSDtCal);
        System.out.println("|[6]RealCalendar(DFormat(7-31-08))   = " + rcDFmtCal);
        System.out.println("|[7]RealCalendar(2008-08-05, SQL)    = " + rcDSqlCal);
        System.out.println("|-------------------------------------------------------------|");
        System.out.println("|Roll 1 day forward");
        System.out.println("|-------------------------------------------------------------|");
        for (int j = 0; j < calendars.size(); ++j)
        {
            RealCalendar c = calendars.get(j);
            c.increment(DateComponent.DAY);
            System.out.println("|["+j+"] = " + c);            
        }
        System.out.println("|-------------------------------------------------------------|");
        System.out.println("|Roll 2 days backwards");
        System.out.println("|-------------------------------------------------------------|");
        for (int j = 0; j < calendars.size(); ++j)
        {
            RealCalendar c = calendars.get(j);
            c.decrement(DateComponent.DAY, 2);
            System.out.println("|["+j+"] = " + c);            
        }
        System.out.println("|-------------------------------------------------------------|");
        System.out.println("|Roll 1 day forward - should now be the same as original");
        System.out.println("|-------------------------------------------------------------|");
        for (int j = 0; j < calendars.size(); ++j)
        {
            RealCalendar c = calendars.get(j);
            c.increment(DateComponent.DAY);
            System.out.println("|["+j+"] = " + c);            
        }
        System.out.println("|-------------------------------------------------------------|");
        System.out.println("|Roll 1 month forward");
        System.out.println("|-------------------------------------------------------------|");
        for (int j = 0; j < calendars.size(); ++j)
        {
            RealCalendar c = calendars.get(j);
            c.increment(DateComponent.MONTH);
            System.out.println("|["+j+"] = " + c);            
        }
        System.out.println("|-------------------------------------------------------------|");
        System.out.println("|Roll 2 months backwards");
        System.out.println("|-------------------------------------------------------------|");
        for (int j = 0; j < calendars.size(); ++j)
        {
            RealCalendar c = calendars.get(j);
            c.decrement(DateComponent.MONTH, 2);
            System.out.println("|["+j+"] = " + c);            
        }
        System.out.println("|-------------------------------------------------------------|");
        System.out.println("|Roll 1 month forward - Should be the same as original");
        System.out.println("|-------------------------------------------------------------|");
        for (int j = 0; j < calendars.size(); ++j)
        {
            RealCalendar c = calendars.get(j);
            c.increment(DateComponent.MONTH);
            System.out.println("|["+j+"] = " + c);            
        }
        System.out.println("|-------------------------------------------------------------|");
        System.out.println("|Roll 1 year forward");
        System.out.println("|-------------------------------------------------------------|");
        for (int j = 0; j < calendars.size(); ++j)
        {
            RealCalendar c = calendars.get(j);
            c.increment(DateComponent.YEAR);
            System.out.println("|["+j+"] = " + c);            
        }
        System.out.println("|-------------------------------------------------------------|");
        System.out.println("|Roll 2 years backwards");
        System.out.println("|-------------------------------------------------------------|");
        for (int j = 0; j < calendars.size(); ++j)
        {
            RealCalendar c = calendars.get(j);
            c.decrement(DateComponent.YEAR, 2);
            System.out.println("|["+j+"] = " + c);            
        }
        System.out.println("|-------------------------------------------------------------|");
        System.out.println("|Roll 1 year forward - should be same as original");
        System.out.println("|-------------------------------------------------------------|");
        for (int j = 0; j < calendars.size(); ++j)
        {
            RealCalendar c = calendars.get(j);
            c.increment(DateComponent.YEAR, 1);
            System.out.println("|["+j+"] = " + c);            
        }
        System.out.println("|-------------------------------------------------------------|");
        System.out.println("|Roll 4 years backwards - should be previous leap year");
        System.out.println("|-------------------------------------------------------------|");
        for (int j = 0; j < calendars.size(); ++j)
        {
            RealCalendar c = calendars.get(j);
            c.decrement(DateComponent.YEAR, 4);
            System.out.println("|["+j+"] = " + c);            
        }
        System.out.println("|-------------------------------------------------------------|");
        System.out.println("|Roll 8 years forward - should be future leap year");
        System.out.println("|-------------------------------------------------------------|");
        for (int j = 0; j < calendars.size(); ++j)
        {
            RealCalendar c = calendars.get(j);
            c.increment(DateComponent.YEAR, 8);
            System.out.println("|["+j+"] = " + c);            
        }
        System.out.println("|-------------------------------------------------------------|");
        System.out.println("|Roll 4 years backwards - should be same as original");
        System.out.println("|-------------------------------------------------------------|");
        for (int j = 0; j < calendars.size(); ++j)
        {
            RealCalendar c = calendars.get(j);
            c.decrement(DateComponent.YEAR, 4);
            System.out.println("|["+j+"] = " + c);            
        }
    }

}
