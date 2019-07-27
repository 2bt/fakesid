# 1. Introduction

Fake SID is a chiptune tracker that let's you create Commodore 64 music.

At the top of the screen you find certain tabs, which let you switch to different views.
Let's go through each view and discuss them in more detail.


# 2. Project

Here you set the title, author, track length, and tempo of the current song.
Additionally, songs can be loaded, saved, deleted, and exported.

*Track length* is the number of rows per track.
All tracks of a song have the same length.
As is common with most C64 trackers, time is split into slices of 1/50 of a second, called frames.
*Tempo* is the number of frames spent per track row.
*Swing* is the number additional frames for even-numbered track rows.

Press *New* to reset the current song.
To load a previously saved song, simply select a song from the song list.
This will enter the song name in the song name input field.
Now press *Load*.
Press *Save* to save the current song under the name in the input field.
Press *Delete* to delete the selected song.
You may render the current song to *WAV* or *OGG* by first selecting the desired file format and then pressing *Export*.
Song files and exported songs are stored in the directories `fakesid/songs` and `fakesid/exports`
of your phone's internal shared storage.


# 3. Song

A song in Fake SID is basically a table with one column per voice.
Table entries are references to tracks.
The original SID - the sound chip of the C64 - has three voices, i.e. you can play three sounds simultaneously.
In Fake SID you have a fourth voice at your disposal, although true purists abstain from using it.
Voices can be muted and unmuted by pressing the relevant buttons in the table header.

Add and remove rows by respectively pressing the buttons labeled *`+`* and *`-`* below the table.
You can set the position at which rows are inserted and deleted by selecting the relevant row index on the left.
This will also set the player position, which is indicated with by highlighted row.

Assign a reference to a table cell by touching it.
This will open up the track select screen with all 252 available track references from `00` to `BK`.
Non-empty tracks are highlighted.
Choose the reference you wish to assign, or touch *Clear* to clear the table cell.

Press and hold a track reference button to switch into the track view of the referenced track.

The three buttons at the bottom are visible in all views.
The first button toggles looping.
Looping causes the player to repeat the current song row indefinitely.
The second button stops playback and resets the player position.
The third button toggles playback.


# 4. Track

Tracks represent one bar of music for one voice.
Tracks are tables with three columns for instrument references, effect references, and notes, in that order.
At the top left you find the reference of the current track.
Touch it to open up the track select screen.
(Alternatively, touch the *Track* tab.)
The arrow buttons let you switch though tracks in sequence.
On the top right are buttons for copying and pasting tracks.

The two rows of buttons below list references to the most recently used instruments and effects, respectively.
Press the corresponding button to select an instrument or effect.
There are 48 slots for instruments and as many for effects.
Press and hold the *Instrument* tab to open up the instrument select screen,
which allows you to select an instrument from among all available instruments.
Selecting effects works the same way.

The main area of the screen shows the track table and the note matrix.
Use the scrollbars to navigate.
Insert notes by touching the respective cell in the note matrix.
This will also assign the selected instrument and effect to the track row.
Changing notes works the same, except that the rows' instrument and effect references are unaffected.
To clear a row, press the note button.
Press it again to insert a note-off event.
Assign and remove an instrument or effect reference from a track row by pressing the relevant button.
To pick up an instrument or effect, press and hold the instrument/effect reference button.


# 5. Instrument

Instruments control the volume and waveform of a voice.
They may also control the filter.

The row of buttons on the top lists references to the most recently used instruments.
Selecting instruments works just like in the track view.
Below, there is the instrument name input field.
Right next to it are buttons for copying and pasting instruments.

Select *Wave* to view the instrument's envelope settings and the wavetable.
Select *Filter* to view the filter table.


## 5.1 Wave

The sliders labeled *Attack*, *Decay*, *Sustain*, and *Release* let you adjust the envelope parameters.
The button on the right toggles hard restart.
Two frames before an instrument with hard restart enabled is triggered,
the voice's gate is cleared and sustain and release are set to zero,
effectively creating a short pause between notes.

The main area of the screen shows the wavetable.
The wavetable defines how the SID control register and the pulse width are updated over time.
Add and remove rows by respectively pressing the buttons labeled *`+`* and *`-`* below the table.
When an instrument is triggered, its wavetable is played, beginning at the top
and progressing to the next row with each frame.
Playback loops after the last row.
Set the loop point by pressing the corresponding row index.

The first four buttons of a wavetable row configure the waveform.
They stand for noise, pulse, sawtooth, and triangle.
Fake SID combines multiple waveforms by binary AND-ing them.
Note that the SID chip is not emulated correctly in this regard.
The next three buttons stand for ring modulation, hardsync, and gate.
The next button specifies the pulse width command, which can be toggled between *`=`* and *`+`*.
The slider on the right sets the command parameter.
The *`=`* command sets the pulse width to the specified value.
The *`+`* command increases the pulse width by the specified amount (scaled down by some factor).
Note that only the pulse wave is affected by the pulse width.


## 5.2 Filter

Just like the original SID chip, Fake SID has one global filter.
Filter parameters are controlled via filter tables.
An instrument's non-empty filter table gets activated any time the instrument is triggered,
replacing the previously active filter table.

The four buttons above the filter table toggle filter routing.
The filter routing configuration is applied with the filter table,
meaning it only takes effect if there's at least one row in the table.

As with wavetables, one row represents one frame.
Setting the loop point and adding and removing rows works the same.

The first three buttons of a filter table row configure the filter type.
They stand for low-pass, band-pass, and high-pass.
The slider next the them sets the resonance.
The next button specifies the cut-off frequency command, which can be toggled between *`+`*, *`=`*, and *`-`*.
The slider on the right sets the command parameter.
The *`=`* command sets the cut-off frequency to the specified value.
The *`-`* and *`+`* commands respectively decrease and increase the cut-off frequency
by the specified amount (scaled down by some factor).


# 6. Effect

Effects modify the pitch of a voice.
They are useful to create arpeggios, vibrato, and percussion sounds.

The row of buttons on the top lists references to the most recently used effects.
Selecting effects works just like in the track view.
Below, there is the effect name input field.
Right next to it are buttons for copying and pasting effects.

As with wavetables, one row represents one frame.
Setting the loop point and adding and removing rows works the same.

Each effect table row has a button that specifies the pitch command,
as well as a slider for the command parameter.
The command button can be toggled between *`+`*, *`~`*, and *`=`*.
The *`+`* command sets a voice's pitch offset in semitones.
The actual pitch of a voice is the sum of this offset and the pitch of the most recent track note.
The *`~`* command works similarly except that the unit is 1/4 of a semitone, which is useful for vibrato and pitch slides.
The *`=`* command sets the absolute pitch, ignoring the note pitch.


# 7. Jam

Play notes live by touching the note matrix.
Selecting instruments and effects works just like in the track view.
Jamming uses the fourth voice,
so expect collisions when there are track notes playing on the fourth voice.
