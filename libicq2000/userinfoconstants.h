/*
 * User Info Constants
 * External constants that clients will use
 *
 * Copyright (C) 2001 Barnaby Gray <barnaby@beedesign.co.uk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 *
 */

#ifndef USERINFOCONSTANTS_H
#define USERINFOCONSTANTS_H

namespace ICQ2000
{
  /*
   * Enums for user info constants
   * Whilst they're all basically just binary compatible with the underlying
   * ICQ binary values, the idea is your client shouldn't be exposed raw
   * numbers, heavens the thought!
   */

enum Sex
{
  SEX_UNKNOWN     = 0,
  SEX_FEMALE      = 1,
  SEX_MALE        = 2
};

enum Timezone
{
  TIMEZONE_UNKNOWN       = -100,
  TIMEZONE_PLUS_12H00M   = -24,
  TIMEZONE_PLUS_11H30M   = -23,
  TIMEZONE_PLUS_11H00M   = -22,
  TIMEZONE_PLUS_10H30M   = -21,
  TIMEZONE_PLUS_10H00M   = -20,
  TIMEZONE_PLUS_09H30M   = -19,
  TIMEZONE_PLUS_09H00M   = -18,
  TIMEZONE_PLUS_08H30M   = -17,
  TIMEZONE_PLUS_08H00M   = -16,
  TIMEZONE_PLUS_07H30M   = -15,
  TIMEZONE_PLUS_07H00M   = -14,
  TIMEZONE_PLUS_06H30M   = -13,
  TIMEZONE_PLUS_06H00M   = -12,
  TIMEZONE_PLUS_05H30M   = -11,
  TIMEZONE_PLUS_05H00M   = -10,
  TIMEZONE_PLUS_04H30M   = -9,
  TIMEZONE_PLUS_04H00M   = -8,
  TIMEZONE_PLUS_03H30M   = -7,
  TIMEZONE_PLUS_03H00M   = -6,
  TIMEZONE_PLUS_02H30M   = -5,
  TIMEZONE_PLUS_02H00M   = -4,
  TIMEZONE_PLUS_01H30M   = -3,
  TIMEZONE_PLUS_01H00M   = -2,
  TIMEZONE_PLUS_00H30M   = -1,
  TIMEZONE_GMT           =  0,
  TIMEZONE_MINUS_00H30M  =  1,
  TIMEZONE_MINUS_01H00M  =  2,
  TIMEZONE_MINUS_01H30M  =  3,
  TIMEZONE_MINUS_02H00M  =  4,
  TIMEZONE_MINUS_02H30M  =  5,
  TIMEZONE_MINUS_03H00M  =  6,
  TIMEZONE_MINUS_03H30M  =  7,
  TIMEZONE_MINUS_04H00M  =  8,
  TIMEZONE_MINUS_04H30M  =  9,
  TIMEZONE_MINUS_05H00M  = 10,
  TIMEZONE_MINUS_05H30M  = 11,
  TIMEZONE_MINUS_06H00M  = 12,
  TIMEZONE_MINUS_06H30M  = 13,
  TIMEZONE_MINUS_07H00M  = 14,
  TIMEZONE_MINUS_07H30M  = 15,
  TIMEZONE_MINUS_08H00M  = 16,
  TIMEZONE_MINUS_08H30M  = 17,
  TIMEZONE_MINUS_09H00M  = 18,
  TIMEZONE_MINUS_09H30M  = 19,
  TIMEZONE_MINUS_10H00M  = 20,
  TIMEZONE_MINUS_10H30M  = 21,
  TIMEZONE_MINUS_11H00M  = 22,
  TIMEZONE_MINUS_11H30M  = 23,
  TIMEZONE_MINUS_12H00M  = 24
};

enum Language
{
  LANGUAGE_UNKNOWN       = 0,
  LANGUAGE_ARABIC        = 1,
  LANGUAGE_BHOJPURI      = 2,
  LANGUAGE_BULGARIAN     = 3,
  LANGUAGE_BURMESE       = 4,
  LANGUAGE_CANTONESE     = 5,
  LANGUAGE_CATALAN       = 6,
  LANGUAGE_CHINESE       = 7,
  LANGUAGE_CROATIAN      = 8,
  LANGUAGE_CZECH         = 9,
  LANGUAGE_DANISH        = 10,
  LANGUAGE_DUTCH         = 11,
  LANGUAGE_ENGLISH       = 12,
  LANGUAGE_ESPERANTO     = 13,
  LANGUAGE_ESTONIAN      = 14,
  LANGUAGE_FARSI         = 15,
  LANGUAGE_FINNISH       = 16,
  LANGUAGE_FRENCH        = 17,
  LANGUAGE_GAELIC        = 18,
  LANGUAGE_GERMAN        = 19,
  LANGUAGE_GREEK         = 20,
  LANGUAGE_HEBREW        = 21,
  LANGUAGE_HINDI         = 22,
  LANGUAGE_HUNGARIAN     = 23,
  LANGUAGE_ICELANDIC     = 24,
  LANGUAGE_INDONESIAN    = 25,
  LANGUAGE_ITALIAN       = 26,
  LANGUAGE_JAPANESE      = 27,
  LANGUAGE_KHMER         = 28,
  LANGUAGE_KOREAN        = 29,
  LANGUAGE_LAO           = 30,
  LANGUAGE_LATVIAN       = 31,
  LANGUAGE_LITHUANIAN    = 32,
  LANGUAGE_MALAY         = 33,
  LANGUAGE_NORWEGIAN     = 34,
  LANGUAGE_POLISH        = 35,
  LANGUAGE_PORTUGUESE    = 36,
  LANGUAGE_ROMANIAN      = 37,
  LANGUAGE_RUSSIAN       = 38,
  LANGUAGE_SERBIAN       = 39,
  LANGUAGE_SLOVAK        = 40,
  LANGUAGE_SLOVENIAN     = 41,
  LANGUAGE_SOMALI        = 42,
  LANGUAGE_SPANISH       = 43,
  LANGUAGE_SWAHILI       = 44,
  LANGUAGE_SWEDISH       = 45,
  LANGUAGE_TAGALOG       = 46,
  LANGUAGE_TATAR         = 47,
  LANGUAGE_THAI          = 48,
  LANGUAGE_TURKISH       = 49,
  LANGUAGE_UKRAINIAN     = 50,
  LANGUAGE_URDU          = 51,
  LANGUAGE_VIETNAMESE    = 52,
  LANGUAGE_YIDDISH       = 53,
  LANGUAGE_YORUBA        = 54,
  LANGUAGE_TAIWANESE     = 55,
  LANGUAGE_AFRIKAANS     = 56,
  LANGUAGE_PERSIAN       = 57,
  LANGUAGE_ALBANIAN      = 58,
  LANGUAGE_ARMENIAN      = 59,
};

enum
{
  LANGUAGE_LIMIT_LOWER = 0,
  LANGUAGE_LIMIT_UPPER = LANGUAGE_ARMENIAN,
};

enum Country
{
  COUNTRY_UNKNOWN                            = 0,
  COUNTRY_AFGHANISTAN                        = 93,
  COUNTRY_ALBANIA                            = 355,
  COUNTRY_ALGERIA                            = 213,
  COUNTRY_AMERICAN_SAMOA                     = 684,
  COUNTRY_ANDORRA                            = 376,
  COUNTRY_ANGOLA                             = 244,
  COUNTRY_ANGUILLA                           = 101,
  COUNTRY_ANTIGUA                            = 102,
  COUNTRY_ARGENTINA                          = 54,
  COUNTRY_ARMENIA                            = 374,
  COUNTRY_ARUBA                              = 297,
  COUNTRY_ASCENSION_ISLAND                   = 247,
  COUNTRY_AUSTRALIA                          = 61,
  COUNTRY_AUSTRALIAN_ANTARCTIC_TERRITORY     = 6721,
  COUNTRY_AUSTRIA                            = 43,
  COUNTRY_AZERBAIJAN                         = 994,
  COUNTRY_BAHAMAS                            = 103,
  COUNTRY_BAHRAIN                            = 973,
  COUNTRY_BANGLADESH                         = 880,
  COUNTRY_BARBADOS                           = 104,
  COUNTRY_BARBUDA                            = 120,
  COUNTRY_BELARUS                            = 375,
  COUNTRY_BELGIUM                            = 32,
  COUNTRY_BELIZE                             = 501,
  COUNTRY_BENIN                              = 229,
  COUNTRY_BERMUDA                            = 105,
  COUNTRY_BHUTAN                             = 975,
  COUNTRY_BOLIVIA                            = 591,
  COUNTRY_BOSNIA_AND_HERZEGOVINA             = 387,
  COUNTRY_BOTSWANA                           = 267,
  COUNTRY_BRAZIL                             = 55,
  COUNTRY_BRITISH_VIRGIN_ISLANDS             = 106,
  COUNTRY_BRUNEI                             = 673,
  COUNTRY_BULGARIA                           = 359,
  COUNTRY_BURKINA_FASO                       = 226,
  COUNTRY_BURUNDI                            = 257,
  COUNTRY_CAMBODIA                           = 855,
  COUNTRY_CAMEROON                           = 237,
  COUNTRY_CANADA                             = 107,
  COUNTRY_CAPE_VERDE_ISLANDS                 = 238,
  COUNTRY_CAYMAN_ISLANDS                     = 108,
  COUNTRY_CENTRAL_AFRICAN_REPUBLIC           = 236,
  COUNTRY_CHAD                               = 235,
  COUNTRY_CHILE                              = 56,
  COUNTRY_CHINA                              = 86,
  COUNTRY_CHRISTMAS_ISLAND                   = 672,
  COUNTRY_COCOS_KEELING_ISLANDS              = 6101,
  COUNTRY_COLOMBIA                           = 57,
  COUNTRY_COMOROS                            = 2691,
  COUNTRY_CONGO                              = 242,
  COUNTRY_COOK_ISLANDS                       = 682,
  COUNTRY_COSTA_RICA                         = 506,
  COUNTRY_CROATIA                            = 385,
  COUNTRY_CUBA                               = 53,
  COUNTRY_CYPRUS                             = 357,
  COUNTRY_CZECH_REPUBLIC                     = 42,
  COUNTRY_DENMARK                            = 45,
  COUNTRY_DIEGO_GARCIA                       = 246,
  COUNTRY_DJIBOUTI                           = 253,
  COUNTRY_DOMINICA                           = 109,
  COUNTRY_DOMINICAN_REPUBLIC                 = 110,
  COUNTRY_ECUADOR                            = 593,
  COUNTRY_EGYPT                              = 20,
  COUNTRY_EL_SALVADOR                        = 503,
  COUNTRY_EQUATORIAL_GUINEA                  = 240,
  COUNTRY_ERITREA                            = 291,
  COUNTRY_ESTONIA                            = 372,
  COUNTRY_ETHIOPIA                           = 251,
  COUNTRY_FAEROE_ISLANDS                     = 298,
  COUNTRY_FALKLAND_ISLANDS                   = 500,
  COUNTRY_FIJI_ISLANDS                       = 679,
  COUNTRY_FINLAND                            = 358,
  COUNTRY_FRANCE                             = 33,
  COUNTRY_FRENCH_ANTILLES                    = 5901,
  COUNTRY_FRENCH_GUIANA                      = 594,
  COUNTRY_FRENCH_POLYNESIA                   = 689,
  COUNTRY_GABON                              = 241,
  COUNTRY_GAMBIA                             = 220,
  COUNTRY_GEORGIA                            = 995,
  COUNTRY_GERMANY                            = 49,
  COUNTRY_GHANA                              = 233,
  COUNTRY_GIBRALTAR                          = 350,
  COUNTRY_GREECE                             = 30,
  COUNTRY_GREENLAND                          = 299,
  COUNTRY_GRENADA                            = 111,
  COUNTRY_GUADELOUPE                         = 590,
  COUNTRY_GUAM                               = 671,
  COUNTRY_GUANTANAMO_BAY                     = 5399,
  COUNTRY_GUATEMALA                          = 502,
  COUNTRY_GUINEA                             = 224,
  COUNTRY_GUINEA_BISSAU                      = 245,
  COUNTRY_GUYANA                             = 592,
  COUNTRY_HAITI                              = 509,
  COUNTRY_HONDURAS                           = 504,
  COUNTRY_HONG_KONG                          = 852,
  COUNTRY_HUNGARY                            = 36,
  COUNTRY_INMARSAT_ATLANTIC_EAST             = 871,
  COUNTRY_INMARSAT_ATLANTIC_WEST             = 874,
  COUNTRY_INMARSAT_INDIAN                    = 873,
  COUNTRY_INMARSAT_PACIFIC                   = 872,
  COUNTRY_INMARSAT                           = 870,
  COUNTRY_ICELAND                            = 354,
  COUNTRY_INDIA                              = 91,
  COUNTRY_INDONESIA                          = 62,
  COUNTRY_INTERNATIONAL_FREEPHONE_SERVICE    = 800,
  COUNTRY_IRAN                               = 98,
  COUNTRY_IRAQ                               = 964,
  COUNTRY_IRELAND                            = 353,
  COUNTRY_ISRAEL                             = 972,
  COUNTRY_ITALY                              = 39,
  COUNTRY_IVORY_COAST                        = 225,
  COUNTRY_JAMAICA                            = 112,
  COUNTRY_JAPAN                              = 81,
  COUNTRY_JORDAN                             = 962,
  COUNTRY_KAZAKHSTAN                         = 705,
  COUNTRY_KENYA                              = 254,
  COUNTRY_KIRIBATI_REPUBLIC                  = 686,
  COUNTRY_KOREA_NORTH                        = 850,
  COUNTRY_KOREA_REPUBLIC_OF                  = 82,
  COUNTRY_KUWAIT                             = 965,
  COUNTRY_KYRGYZ_REPUBLIC                    = 706,
  COUNTRY_LAOS                               = 856,
  COUNTRY_LATVIA                             = 371,
  COUNTRY_LEBANON                            = 961,
  COUNTRY_LESOTHO                            = 266,
  COUNTRY_LIBERIA                            = 231,
  COUNTRY_LIBYA                              = 218,
  COUNTRY_LIECHTENSTEIN                      = 4101,
  COUNTRY_LITHUANIA                          = 370,
  COUNTRY_LUXEMBOURG                         = 352,
  COUNTRY_MACAU                              = 853,
  COUNTRY_MADAGASCAR                         = 261,
  COUNTRY_MALAWI                             = 265,
  COUNTRY_MALAYSIA                           = 60,
  COUNTRY_MALDIVES                           = 960,
  COUNTRY_MALI                               = 223,
  COUNTRY_MALTA                              = 356,
  COUNTRY_MARSHALL_ISLANDS                   = 692,
  COUNTRY_MARTINIQUE                         = 596,
  COUNTRY_MAURITANIA                         = 222,
  COUNTRY_MAURITIUS                          = 230,
  COUNTRY_MAYOTTE_ISLAND                     = 269,
  COUNTRY_MEXICO                             = 52,
  COUNTRY_MICRONESIA_FEDERATED_STATES_OF     = 691,
  COUNTRY_MOLDOVA                            = 373,
  COUNTRY_MONACO                             = 377,
  COUNTRY_MONGOLIA                           = 976,
  COUNTRY_MONTSERRAT                         = 113,
  COUNTRY_MOROCCO                            = 212,
  COUNTRY_MOZAMBIQUE                         = 258,
  COUNTRY_MYANMAR                            = 95,
  COUNTRY_NAMIBIA                            = 264,
  COUNTRY_NAURU                              = 674,
  COUNTRY_NEPAL                              = 977,
  COUNTRY_NETHERLANDS_ANTILLES               = 599,
  COUNTRY_NETHERLANDS                        = 31,
  COUNTRY_NEVIS                              = 114,
  COUNTRY_NEW_CALEDONIA                      = 687,
  COUNTRY_NEW_ZEALAND                        = 64,
  COUNTRY_NICARAGUA                          = 505,
  COUNTRY_NIGER                              = 227,
  COUNTRY_NIGERIA                            = 234,
  COUNTRY_NIUE                               = 683,
  COUNTRY_NORFOLK_ISLAND                     = 6722,
  COUNTRY_NORWAY                             = 47,
  COUNTRY_OMAN                               = 968,
  COUNTRY_PAKISTAN                           = 92,
  COUNTRY_PALAU                              = 680,
  COUNTRY_PANAMA                             = 507,
  COUNTRY_PAPUA_NEW_GUINEA                   = 675,
  COUNTRY_PARAGUAY                           = 595,
  COUNTRY_PERU                               = 51,
  COUNTRY_PHILIPPINES                        = 63,
  COUNTRY_POLAND                             = 48,
  COUNTRY_PORTUGAL                           = 351,
  COUNTRY_PUERTO_RICO                        = 121,
  COUNTRY_QATAR                              = 974,
  COUNTRY_REPUBLIC_OF_MACEDONIA              = 389,
  COUNTRY_REUNION_ISLAND                     = 262,
  COUNTRY_ROMANIA                            = 40,
  COUNTRY_ROTA_ISLAND                        = 6701,
  COUNTRY_RUSSIA                             = 7,
  COUNTRY_RWANDA                             = 250,
  COUNTRY_SAINT_LUCIA                        = 122,
  COUNTRY_SAIPAN_ISLAND                      = 670,
  COUNTRY_SAN_MARINO                         = 378,
  COUNTRY_SAO_TOME_AND_PRINCIPE              = 239,
  COUNTRY_SAUDI_ARABIA                       = 966,
  COUNTRY_SENEGAL_REPUBLIC                   = 221,
  COUNTRY_SEYCHELLE_ISLANDS                  = 248,
  COUNTRY_SIERRA_LEONE                       = 232,
  COUNTRY_SINGAPORE                          = 65,
  COUNTRY_SLOVAK_REPUBLIC                    = 4201,
  COUNTRY_SLOVENIA                           = 386,
  COUNTRY_SOLOMON_ISLANDS                    = 677,
  COUNTRY_SOMALIA                            = 252,
  COUNTRY_SOUTH_AFRICA                       = 27,
  COUNTRY_SPAIN                              = 34,
  COUNTRY_SRI_LANKA                          = 94,
  COUNTRY_ST_HELENA                          = 290,
  COUNTRY_ST_KITTS                           = 115,
  COUNTRY_ST_PIERRE_AND_MIQUELON             = 508,
  COUNTRY_ST_VINCENT_AND_THE_GRENADINES      = 116,
  COUNTRY_SUDAN                              = 249,
  COUNTRY_SURINAME                           = 597,
  COUNTRY_SWAZILAND                          = 268,
  COUNTRY_SWEDEN                             = 46,
  COUNTRY_SWITZERLAND                        = 41,
  COUNTRY_SYRIA                              = 963,
  COUNTRY_TAIWAN_REPUBLIC_OF_CHINA           = 886,
  COUNTRY_TAJIKISTAN                         = 708,
  COUNTRY_TANZANIA                           = 255,
  COUNTRY_THAILAND                           = 66,
  COUNTRY_TINIAN_ISLAND                      = 6702,
  COUNTRY_TOGO                               = 228,
  COUNTRY_TOKELAU                            = 690,
  COUNTRY_TONGA                              = 676,
  COUNTRY_TRINIDAD_AND_TOBAGO                = 117,
  COUNTRY_TUNISIA                            = 216,
  COUNTRY_TURKEY                             = 90,
  COUNTRY_TURKMENISTAN                       = 709,
  COUNTRY_TURKS_AND_CAICOS_ISLANDS           = 118,
  COUNTRY_TUVALU                             = 688,
  COUNTRY_USA                                = 1,
  COUNTRY_UGANDA                             = 256,
  COUNTRY_UKRAINE                            = 380,
  COUNTRY_UNITED_ARAB_EMIRATES               = 971,
  COUNTRY_UNITED_KINGDOM                     = 44,
  COUNTRY_UNITED_STATES_VIRGIN_ISLANDS       = 123,
  COUNTRY_URUGUAY                            = 598,
  COUNTRY_UZBEKISTAN                         = 711,
  COUNTRY_VANUATU                            = 678,
  COUNTRY_VATICAN_CITY                       = 379,
  COUNTRY_VENEZUELA                          = 58,
  COUNTRY_VIETNAM                            = 84,
  COUNTRY_WALLIS_AND_FUTUNA_ISLANDS          = 681,
  COUNTRY_WESTERN_SAMOA                      = 685,
  COUNTRY_YEMEN                              = 967,
  COUNTRY_YUGOSLAVIA                         = 381,
  COUNTRY_ZAIRE                              = 243,
  COUNTRY_ZAMBIA                             = 260,
  COUNTRY_ZIMBABWE                           = 263,
};
 
enum Interest
{
  INTEREST_ART                     = 100,
  INTEREST_CARS                    = 101,
  INTEREST_CELEBRITY_FANS          = 102,
  INTEREST_COLLECTIONS             = 103,
  INTEREST_COMPUTERS               = 104,
  INTEREST_CULTURE                 = 105,
  INTEREST_FITNESS                 = 106,
  INTEREST_GAMES                   = 107,
  INTEREST_HOBBIES                 = 108,
  INTEREST_ICQ___HELP              = 109,
  INTEREST_INTERNET                = 110,
  INTEREST_LIFESTYLE               = 111,
  INTEREST_MOVIES_AND_TV           = 112,
  INTEREST_MUSIC                   = 113,
  INTEREST_OUTDOORS                = 114,
  INTEREST_PARENTING               = 115,
  INTEREST_PETS_AND_ANIMALS        = 116,
  INTEREST_RELIGION                = 117,
  INTEREST_SCIENCE                 = 118,
  INTEREST_SKILLS                  = 119,
  INTEREST_SPORTS                  = 120,
  INTEREST_WEB_DESIGN              = 121,
  INTEREST_ECOLOGY                 = 122,
  INTEREST_NEWS_AND_MEDIA          = 123,
  INTEREST_GOVERNMENT              = 124,
  INTEREST_BUSINESS                = 125,
  INTEREST_MYSTICS                 = 126,
  INTEREST_TRAVEL                  = 127,
  INTEREST_ASTRONOMY               = 128,
  INTEREST_SPACE                   = 129,
  INTEREST_CLOTHING                = 130,
  INTEREST_PARTIES                 = 131,
  INTEREST_WOMEN                   = 132,
  INTEREST_SOCIAL_SCIENCE          = 133,
  INTEREST_60S                     = 134,
  INTEREST_70S                     = 135,
  INTEREST_80S                     = 136,
  INTEREST_50S                     = 137,
  INTEREST_FINANCE_AND_CORPORATE   = 138,
  INTEREST_ENTERTAINMENT           = 139,
  INTEREST_CONSUMER_ELECTRONICS    = 140,
  INTEREST_RETAIL_STORES           = 141,
  INTEREST_HEALTH_AND_BEAUTY       = 142,
  INTEREST_MEDIA                   = 143,
  INTEREST_HOUSEHOLD_PRODUCTS      = 144,
  INTEREST_MAIL_ORDER_CATALOG      = 145,
  INTEREST_BUSINESS_SERVICES       = 146,
  INTEREST_AUDIO_AND_VISUAL        = 147,
  INTEREST_SPORTING_AND_ATHLETICS  = 148,
  INTEREST_PUBLISHING              = 149,
  INTEREST_HOME_AUTOMATION         = 150,
};

enum Background
{
  BACKGROUND_ELEMENTARY_SCHOOL     = 300,
  BACKGROUND_HIGH_SCHOOL           = 301,
  BACKGROUND_COLLEGE               = 302,
  BACKGROUND_UNIVERSITY            = 303,
  BACKGROUND_MILITARY              = 304,
  BACKGROUND_PAST_WORK_PLACE       = 305,
  BACKGROUND_PAST_ORGANIZATION     = 306,
  BACKGROUND_OTHER                 = 399,
};


enum AgeRange
{
  RANGE_NORANGE  = 0,
  RANGE_18_22    = 1,
  RANGE_23_29    = 2,
  RANGE_30_39    = 3,
  RANGE_40_49    = 4,
  RANGE_50_59    = 5,
  RANGE_60_ABOVE = 6,
};

}

#endif
