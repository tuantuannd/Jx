object frmAttrChange: TfrmAttrChange
  Left = 413
  Top = 152
  BorderStyle = bsDialog
  Caption = #25991#20214#23646#24615#35774#32622
  ClientHeight = 456
  ClientWidth = 418
  Color = clBtnFace
  Font.Charset = GB2312_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = #23435#20307
  Font.Style = []
  OldCreateOrder = False
  Position = poOwnerFormCenter
  PixelsPerInch = 96
  TextHeight = 12
  object Label1: TLabel
    Left = 8
    Top = 12
    Width = 129
    Height = 13
    Alignment = taRightJustify
    AutoSize = False
    Caption = #25991#20214#21517#65306
  end
  object Label2: TLabel
    Left = 8
    Top = 36
    Width = 129
    Height = 13
    Alignment = taRightJustify
    AutoSize = False
    Caption = #25991#20214#36335#24452#65306
  end
  object Label3: TLabel
    Left = 8
    Top = 68
    Width = 129
    Height = 13
    Alignment = taRightJustify
    AutoSize = False
    Caption = 'Belong'#65306
  end
  object Label4: TLabel
    Left = 0
    Top = 88
    Width = 408
    Height = 13
    Alignment = taRightJustify
    AutoSize = False
    Caption = #25991#20214#36866#29992#20110#30340#26412#22320#29256#26412#65292'<0'#25152#26377#26412#22320#29256#26412', >=0'#29305#23450#26412#22320#29256#26412
  end
  object Label5: TLabel
    Left = 8
    Top = 116
    Width = 129
    Height = 13
    Alignment = taRightJustify
    AutoSize = False
    Caption = 'CheckVersionMethod'#65306
  end
  object Label6: TLabel
    Left = 0
    Top = 136
    Width = 408
    Height = 25
    Alignment = taRightJustify
    AutoSize = False
    Caption = #21028#26029#26159#21542#35201#26356#26032#30340#26041#27861#65306#10#13'5'#65293#65293#25991#26412#25991#20214#65292#27809#26377#29256#26412#20449#24687#65292#21462#25991#20214#30340#20462#25913#26102#38388#20316#20026#29256#26412#21495
  end
  object Label7: TLabel
    Left = 8
    Top = 172
    Width = 129
    Height = 13
    Alignment = taRightJustify
    AutoSize = False
    Caption = 'UpdateFileFlag'#65306
  end
  object Label8: TLabel
    Left = 0
    Top = 192
    Width = 408
    Height = 13
    Alignment = taRightJustify
    AutoSize = False
    Caption = #35813'Section'#20026#26356#26032#25991#20214#20449#24687#30340#26631#24535
  end
  object Label9: TLabel
    Left = 8
    Top = 220
    Width = 129
    Height = 13
    Alignment = taRightJustify
    AutoSize = False
    Caption = 'LocalPath'#65306
  end
  object Label10: TLabel
    Left = 0
    Top = 240
    Width = 408
    Height = 17
    Alignment = taRightJustify
    AutoSize = False
    Caption = #26412#22320#26356#26032#30446#24405#65306'%PROGRAMDIR%'#12289'%PROGRAMDIR\XXX\'#12289'%SYSTEMDIR%'#12289'%WINDIR%'
  end
  object Label11: TLabel
    Left = 0
    Top = 288
    Width = 408
    Height = 25
    Alignment = taRightJustify
    AutoSize = False
    Caption = 
      #26356#26032#25991#20214#30340#26041#27861' Copy;Last;Package'#21508#20010#21160#20316#20043#38388#38548#24320#22914'Last;Copy'#10#13#26356#26032#21253#65306'Package(Update.' +
      'pak)'
  end
  object Label12: TLabel
    Left = 8
    Top = 268
    Width = 129
    Height = 13
    Alignment = taRightJustify
    AutoSize = False
    Caption = 'UpdateFileMethod'#65306
  end
  object Label13: TLabel
    Left = 0
    Top = 352
    Width = 408
    Height = 13
    Alignment = taRightJustify
    AutoSize = False
    Caption = #36828#31243#30340#30456#23545#36335#24452#65288#30456#23545#25552#20379#30340#19979#36733#36335#24452#65289
  end
  object Label14: TLabel
    Left = 8
    Top = 332
    Width = 129
    Height = 13
    Alignment = taRightJustify
    AutoSize = False
    Caption = 'RemotePath'#65306
  end
  object Label15: TLabel
    Left = 0
    Top = 400
    Width = 408
    Height = 13
    Alignment = taRightJustify
    AutoSize = False
    Caption = #38656#35201#20248#20808#26356#26032#30340#26631#24535
  end
  object Label16: TLabel
    Left = 8
    Top = 380
    Width = 129
    Height = 13
    Alignment = taRightJustify
    AutoSize = False
    Caption = 'UpdateRelative'#65306
  end
  object txtFileName: TEdit
    Left = 136
    Top = 8
    Width = 273
    Height = 20
    ImeName = #20013#25991' ('#31616#20307') - '#24494#36719#25340#38899
    TabOrder = 0
  end
  object txtFilePath: TEdit
    Left = 136
    Top = 32
    Width = 273
    Height = 20
    ImeName = #20013#25991' ('#31616#20307') - '#24494#36719#25340#38899
    TabOrder = 1
  end
  object txtBelong: TEdit
    Left = 136
    Top = 64
    Width = 273
    Height = 20
    ImeName = #20013#25991' ('#31616#20307') - '#24494#36719#25340#38899
    TabOrder = 2
  end
  object txtCheckVersionMethod: TEdit
    Left = 136
    Top = 112
    Width = 273
    Height = 20
    ImeName = #20013#25991' ('#31616#20307') - '#24494#36719#25340#38899
    TabOrder = 3
  end
  object txtUpdateFileFlag: TEdit
    Left = 136
    Top = 168
    Width = 273
    Height = 20
    ImeName = #20013#25991' ('#31616#20307') - '#24494#36719#25340#38899
    TabOrder = 4
  end
  object txtLocalPath: TEdit
    Left = 136
    Top = 216
    Width = 273
    Height = 20
    ImeName = #20013#25991' ('#31616#20307') - '#24494#36719#25340#38899
    TabOrder = 5
  end
  object txtUpdateFileMethod: TEdit
    Left = 136
    Top = 264
    Width = 273
    Height = 20
    ImeName = #20013#25991' ('#31616#20307') - '#24494#36719#25340#38899
    TabOrder = 6
  end
  object txtRemotePath: TEdit
    Left = 136
    Top = 328
    Width = 273
    Height = 20
    ImeName = #20013#25991' ('#31616#20307') - '#24494#36719#25340#38899
    TabOrder = 7
  end
  object txtUpdateRelative: TEdit
    Left = 136
    Top = 376
    Width = 273
    Height = 20
    ImeName = #20013#25991' ('#31616#20307') - '#24494#36719#25340#38899
    TabOrder = 8
  end
  object btnOK: TButton
    Left = 232
    Top = 424
    Width = 75
    Height = 23
    Caption = #30830#23450'(&O)'
    TabOrder = 9
    OnClick = btnOKClick
  end
  object btnCancel: TButton
    Left = 328
    Top = 424
    Width = 75
    Height = 23
    Caption = #21462#28040'(&C)'
    TabOrder = 10
    OnClick = btnCancelClick
  end
end
