/*
 * Copyright (C) 2014  mingspy@163.com
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <iostream>
#include <string>
#include <vector>
#include "builder.hpp"

using namespace std;
int main(int argc, char ** argv)
{
    mingspy::Builder builder;
    if(argc > 1) {
        cout<<"building inverse dictionary"<<endl;
        builder.setInverse(true);
    } else {
        cout<<"building core dictionary"<<endl;
    }
    vector<string> files;
    listFiles("../data/people/",files);
    builder.buildFromPeopleDaily(files);
    builder.load_udf_dict("../data/dicts/main.txt", "UDF");
    builder.load_udf_dict("../data/dicts/persons.txt", "nr");
    builder.load_udf_dict("../data/dicts/companies.txt", "nt");
    if(argc > 1) {
        builder.save("./inverse.dic");
    } else {
        builder.save("./core.dic");
    }
    return 0;
}
