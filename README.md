#  pd-externals

### polyblep~
External module for use in Pure Data that outputs a sawtooth wave generated with PolyBLEPs. The waveform retains much of its harmonic content, giving it a rich and deep sound. While the waveform is not completely band-limited, aliasing noise is very minimal if not entirely inaudible in all but the highest frequencies approaching Nyquist.

![](https://github.com/cfloisand/pd-externals/blob/master/patch_polyblep.png "polyblep~ patch")

### foldback~
External module for use in Pure Data that applies foldback distortion to an incoming signal.

![](https://github.com/cfloisand/pd-externals/blob/master/patch_foldback.png "foldback~ patch")

## Installation
The generated libraries should be placed in the following folders based on platform (or whatever folder location is specified in Pure Data's externals path):

#### macOS
`~/Library/Pd`
or
`/Library/Pd`

#### Windows
`%appdata%\Pd`
or
`%userprofile%\Documents\Pd\externals`

To access a module's help patch, copy the patches in the `Patches` directory into the `doc/5.reference/` directory of the Pure Data application.

## License
pd-externals is released under the MIT license. See [LICENSE](https://github.com/cfloisand/pd-externals/blob/master/LICENSE.txt) for more details.

---

Pure Data, Copyright (c) 1997-1999 Miller Puckette.

This program is free software: you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation, either
version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the  GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program.
If not, see <http://www.gnu.org/licenses/>.
