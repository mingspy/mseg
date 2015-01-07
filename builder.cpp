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
#include "utils.hpp"

using namespace std;
int main(int argc, char ** argv)
{
    mingspy::Builder builder;
    vector<string> files;
    getFiles("../data/people/",files);
    cout<<"here:31"<<endl;
    if(argc > 1)
    {
        builder.setInverse(true);
        builder.buildFromPeopleDaily(files,"./inverse.dic");
    }
    else{
        cout<<"here:38"<<endl;
        builder.buildFromPeopleDaily(files,"./core.dic");
    }
    return 0;
}
