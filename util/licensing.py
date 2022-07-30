
import os, re

license = """
/*
 * Copyright (C) 2022  SCS Lab <scslab@iit.edu>,
 * Luke Logan <llogan@hawk.iit.edu>,
 * Jaime Cernuda Garcia <jcernudagarcia@hawk.iit.edu>
 * Jay Lofstead <gflofst@sandia.gov>,
 * Anthony Kougkas <akougkas@iit.edu>,
 * Xian-He Sun <sun@iit.edu>
 *
 * This file is part of LabStor
 *
 * LabStor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

"""

preamble_test = """
//
// Created by lukemartinlogan on 9/7/21.
//


int main() {


}
"""

from jarvis_cd.serialize.text_file import TextFile

def ErasePreamble(text):
    lines = text.splitlines()
    for id,line in enumerate(lines):
        if 'Created by lukemartinlogan on' in line:
            del lines[id-1]
            del lines[id-1]
            del lines[id-1]
    return "\n".join(lines).strip()

def Paths(root):
    filenames = os.listdir(root)
    for filename in filenames:
        if os.path.isfile(os.path.join(root,filename)):
            if filename.endswith('.cpp') or filename.endswith('.h') or filename.endswith('.c'):
                print(os.path.join(root, filename))
                node = TextFile(os.path.join(root,filename))
                text = node.Load()
                text = ErasePreamble(text)
                text = license + text
                node.Save(text)
        else:
            Paths(os.path.join(root,filename))

Paths(os.getcwd())