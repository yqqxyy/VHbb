As of 14 Nov, `CxAODReader` and `CxAODReader_VHbb` have their pipelines updated by Andreas Hoenle to impose a coding style and reformat our code if it is in a different style to the new style. 

Info in the 3rd page of these [slides](https://indico.cern.ch/event/741272/contributions/3099517/attachments/1698502/2734536/20180807_andreas.pdf). A video linked in the slides is really recommended.

Our style file is [here](https://gitlab.cern.ch/ahoenle/atlas-clang-format/blob/master/style/clang-format.style). It is basically the Google style guide, except that we don't require whitespaces after comments (for `ROOT` `linting`), e.g.
```
foo m_myMember; //!
```
will not become
```
foo m_myMember; // !
```

