/*
 * User Info Constants
 * External constants that clients will use
 * Copyright (C) 2001 Barnaby Gray <barnaby@beedesign.co.uk>.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef USERINFOCONSTANTS_H
#define USERINFOCONSTANTS_H

static const unsigned char Language_table_size = 60;

static const char* Language_table[Language_table_size] = {
  "Unspecified",
  "Arabic",
  "Bhojpuri",
  "Bulgarian",
  "Burmese",
  "Cantonese",
  "Catalan",
  "Chinese",
  "Croatian",
  "Czech",
  "Danish",
  "Dutch",
  "English",
  "Esperanto",
  "Estonian",
  "Farsi",
  "Finnish",
  "French",
  "Gaelic",
  "German",
  "Greek",
  "Hebrew",
  "Hindi",
  "Hungarian",
  "Icelandic",
  "Indonesian",
  "Italian",
  "Japanese",
  "Khmer",
  "Korean",
  "Lao",
  "Latvian",
  "Lithuanian",
  "Malay",
  "Norwegian",
  "Polish",
  "Portuguese",
  "Romanian",
  "Russian",
  "Serbian",
  "Slovak",
  "Slovenian",
  "Somali",
  "Spanish",
  "Swahili",
  "Swedish",
  "Tagalog",
  "Tatar",
  "Thai",
  "Turkish",
  "Ukrainian",
  "Urdu",
  "Vietnamese",
  "Yiddish",
  "Yoruba",
  "Taiwanese",
  "Afrikaans",
  "Persian",
  "Albanian",
  "Armenian",
};


struct Country
{
  char *name;          /* Name of the country */
  unsigned short code; /* Country code */
};

static const unsigned short Country_table_size = 243;

static const struct Country Country_table[Country_table_size] =
{
  { "Unspecified", 0 },
  { "Afghanistan", 93 },
  { "Albania", 355 },
  { "Algeria", 213 },
  { "American Samoa", 684 },
  { "Andorra", 376 },
  { "Angola", 244 },
  { "Anguilla", 101 },
  { "Antigua", 102 },
  { "Argentina", 54 },
  { "Armenia", 374 },
  { "Aruba", 297 },
  { "Ascension Island", 247 },
  { "Australia", 61 },
  { "Australian Antarctic Territory", 6721 },
  { "Austria", 43 },
  { "Azerbaijan", 994 },
  { "Bahamas", 103 },
  { "Bahrain", 973 },
  { "Bangladesh", 880 },
  { "Barbados", 104 },
  { "Barbuda", 120 },
  { "Belarus", 375 },
  { "Belgium", 32 },
  { "Belize", 501 },
  { "Benin", 229 },
  { "Bermuda", 105 },
  { "Bhutan", 975 },
  { "Bolivia", 591 },
  { "Bosnia and Herzegovina", 387 },
  { "Botswana", 267 },
  { "Brazil", 55 },
  { "British Virgin Islands", 106 },
  { "Brunei", 673 },
  { "Bulgaria", 359 },
  { "Burkina Faso", 226 },
  { "Burundi", 257 },
  { "Cambodia", 855 },
  { "Cameroon", 237 },
  { "Canada", 107 },
  { "Cape Verde Islands", 238 },
  { "Cayman Islands", 108 },
  { "Central African Republic", 236 },
  { "Chad", 235 },
  { "Chile", 56 },
  { "China", 86 },
  { "Christmas Island", 672 },
  { "Cocos-Keeling Islands", 6101 },
  { "Colombia", 57 },
  { "Comoros", 2691 },
  { "Congo", 242 },
  { "Cook Islands", 682 },
  { "Costa Rica", 506 },
  { "Croatia", 385 },
  { "Cuba", 53 },
  { "Cyprus", 357 },
  { "Czech Republic", 42 },
  { "Denmark", 45 },
  { "Diego Garcia", 246 },
  { "Djibouti", 253 },
  { "Dominica", 109 },
  { "Dominican Republic", 110 },
  { "Ecuador", 593 },
  { "Egypt", 20 },
  { "El Salvador", 503 },
  { "Equatorial Guinea", 240 },
  { "Eritrea", 291 },
  { "Estonia", 372 },
  { "Ethiopia", 251 },
  { "Faeroe Islands", 298 },
  { "Falkland Islands", 500 },
  { "Fiji Islands", 679 },
  { "Finland", 358 },
  { "France", 33 },
  { "French Antilles", 5901 },
  { "French Guiana", 594 },
  { "French Polynesia", 689 },
  { "Gabon", 241 },
  { "Gambia", 220 },
  { "Georgia", 995 },
  { "Germany", 49 },
  { "Ghana", 233 },
  { "Gibraltar", 350 },
  { "Greece", 30 },
  { "Greenland", 299 },
  { "Grenada", 111 },
  { "Guadeloupe", 590 },
  { "Guam", 671 },
  { "Guantanamo Bay", 5399 },
  { "Guatemala", 502 },
  { "Guinea", 224 },
  { "Guinea-Bissau", 245 },
  { "Guyana", 592 },
  { "Haiti", 509 },
  { "Honduras", 504 },
  { "Hong Kong", 852 },
  { "Hungary", 36 },
  { "INMARSAT (Atlantic-East)", 871 },
  { "INMARSAT (Atlantic-West)", 874 },
  { "INMARSAT (Indian)", 873 },
  { "INMARSAT (Pacific)", 872 },
  { "INMARSAT", 870 },
  { "Iceland", 354 },
  { "India", 91 },
  { "Indonesia", 62 },
  { "International Freephone Service", 800 },
  { "Iran", 98 },
  { "Iraq", 964 },
  { "Ireland", 353 },
  { "Israel", 972 },
  { "Italy", 39 },
  { "Ivory Coast", 225 },
  { "Jamaica", 112 },
  { "Japan", 81 },
  { "Jordan", 962 },
  { "Kazakhstan", 705 },
  { "Kenya", 254 },
  { "Kiribati Republic", 686 },
  { "Korea (North)", 850 },
  { "Korea (Republic of)", 82 },
  { "Kuwait", 965 },
  { "Kyrgyz Republic", 706 },
  { "Laos", 856 },
  { "Latvia", 371 },
  { "Lebanon", 961 },
  { "Lesotho", 266 },
  { "Liberia", 231 },
  { "Libya", 218 },
  { "Liechtenstein", 4101 },
  { "Lithuania", 370 },
  { "Luxembourg", 352 },
  { "Macau", 853 },
  { "Madagascar", 261 },
  { "Malawi", 265 },
  { "Malaysia", 60 },
  { "Maldives", 960 },
  { "Mali", 223 },
  { "Malta", 356 },
  { "Marshall Islands", 692 },
  { "Martinique", 596 },
  { "Mauritania", 222 },
  { "Mauritius", 230 },
  { "Mayotte Island", 269 },
  { "Mexico", 52 },
  { "Micronesia, Federated States of", 691 },
  { "Moldova", 373 },
  { "Monaco", 377 },
  { "Mongolia", 976 },
  { "Montserrat", 113 },
  { "Morocco", 212 },
  { "Mozambique", 258 },
  { "Myanmar", 95 },
  { "Namibia", 264 },
  { "Nauru", 674 },
  { "Nepal", 977 },
  { "Netherlands Antilles", 599 },
  { "Netherlands", 31 },
  { "Nevis", 114 },
  { "New Caledonia", 687 },
  { "New Zealand", 64 },
  { "Nicaragua", 505 },
  { "Niger", 227 },
  { "Nigeria", 234 },
  { "Niue", 683 },
  { "Norfolk Island", 6722 },
  { "Norway", 47 },
  { "Oman", 968 },
  { "Pakistan", 92 },
  { "Palau", 680 },
  { "Panama", 507 },
  { "Papua New Guinea", 675 },
  { "Paraguay", 595 },
  { "Peru", 51 },
  { "Philippines", 63 },
  { "Poland", 48 },
  { "Portugal", 351 },
  { "Puerto Rico", 121 },
  { "Qatar", 974 },
  { "Republic of Macedonia", 389 },
  { "Reunion Island", 262 },
  { "Romania", 40 },
  { "Rota Island", 6701 },
  { "Russia", 7 },
  { "Rwanda", 250 },
  { "Saint Lucia", 122 },
  { "Saipan Island", 670 },
  { "San Marino", 378 },
  { "Sao Tome and Principe", 239 },
  { "Saudi Arabia", 966 },
  { "Senegal Republic", 221 },
  { "Seychelle Islands", 248 },
  { "Sierra Leone", 232 },
  { "Singapore", 65 },
  { "Slovak Republic", 4201 },
  { "Slovenia", 386 },
  { "Solomon Islands", 677 },
  { "Somalia", 252 },
  { "South Africa", 27 },
  { "Spain", 34 },
  { "Sri Lanka", 94 },
  { "St. Helena", 290 },
  { "St. Kitts", 115 },
  { "St. Pierre and Miquelon", 508 },
  { "St. Vincent and the Grenadines", 116 },
  { "Sudan", 249 },
  { "Suriname", 597 },
  { "Swaziland", 268 },
  { "Sweden", 46 },
  { "Switzerland", 41 },
  { "Syria", 963 },
  { "Taiwan, Republic of China", 886 },
  { "Tajikistan", 708 },
  { "Tanzania", 255 },
  { "Thailand", 66 },
  { "Tinian Island", 6702 },
  { "Togo", 228 },
  { "Tokelau", 690 },
  { "Tonga", 676 },
  { "Trinidad and Tobago", 117 },
  { "Tunisia", 216 },
  { "Turkey", 90 },
  { "Turkmenistan", 709 },
  { "Turks and Caicos Islands", 118 },
  { "Tuvalu", 688 },
  { "USA", 1 },
  { "Uganda", 256 },
  { "Ukraine", 380 },
  { "United Arab Emirates", 971 },
  { "United Kingdom", 44 },
  { "United States Virgin Islands", 123 },
  { "Uruguay", 598 },
  { "Uzbekistan", 711 },
  { "Vanuatu", 678 },
  { "Vatican City", 379 },
  { "Venezuela", 58 },
  { "Vietnam", 84 },
  { "Wallis and Futuna Islands", 681 },
  { "Western Samoa", 685 },
  { "Yemen", 967 },
  { "Yugoslavia", 381 },
  { "Zaire", 243 },
  { "Zambia", 260 },
  { "Zimbabwe", 263 }
};

#endif
