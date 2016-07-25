object frmMakeUpdate: TfrmMakeUpdate
  Left = 281
  Top = 241
  BorderStyle = bsDialog
  Caption = 'frmMakeUpdate'
  ClientHeight = 380
  ClientWidth = 583
  Color = clBtnFace
  Font.Charset = GB2312_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = #23435#20307
  Font.Style = []
  OldCreateOrder = False
  Position = poOwnerFormCenter
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 12
  object alabMainVer: TLabel
    Left = 8
    Top = 12
    Width = 48
    Height = 12
    Caption = #20027#29256#26412#65306
  end
  object alabPackBase: TLabel
    Left = 160
    Top = 12
    Width = 84
    Height = 12
    Caption = #20027#29256#26412#36164#28304#21253#65306
  end
  object alabSaveDir: TLabel
    Left = 8
    Top = 92
    Width = 96
    Height = 12
    Caption = #26356#26032#21253#20445#23384#30446#24405#65306
  end
  object alabUpdateName: TLabel
    Left = 32
    Top = 68
    Width = 72
    Height = 12
    Caption = #26356#26032#21253#21517#31216#65306
  end
  object alabSubVer: TLabel
    Left = 8
    Top = 36
    Width = 48
    Height = 12
    Caption = #21103#29256#26412#65306
  end
  object alabPackAdd: TLabel
    Left = 160
    Top = 36
    Width = 84
    Height = 12
    Caption = #21103#29256#26412#36164#28304#21253#65306
  end
  object txtMainVer: TEdit
    Left = 56
    Top = 8
    Width = 73
    Height = 20
    Enabled = False
    ImeName = #20013#25991' ('#31616#20307') - '#24494#36719#25340#38899
    TabOrder = 0
  end
  object txtPackBase: TEdit
    Left = 248
    Top = 8
    Width = 281
    Height = 20
    Enabled = False
    ImeName = #20013#25991' ('#31616#20307') - '#24494#36719#25340#38899
    TabOrder = 1
  end
  object lvUpdatePack: TListView
    Left = 8
    Top = 120
    Width = 569
    Height = 209
    Checkboxes = True
    Columns = <
      item
        Caption = #25991#20214#21517
        Width = 130
      end
      item
        Caption = #29256#26412':'#20174
        Width = 60
      end
      item
        Caption = #29256#26412':'#21040
        Width = 60
      end
      item
        Caption = #36335#24452
        Width = 300
      end
      item
        Caption = #21629#20196
        Width = 500
      end>
    ColumnClick = False
    ReadOnly = True
    RowSelect = True
    TabOrder = 2
    ViewStyle = vsReport
  end
  object txtSaveDir: TEdit
    Left = 104
    Top = 88
    Width = 345
    Height = 20
    ImeName = #20013#25991' ('#31616#20307') - '#24494#36719#25340#38899
    TabOrder = 3
    OnChange = txtMainTextChange
  end
  object btnSaveDir: TButton
    Left = 456
    Top = 88
    Width = 25
    Height = 20
    Caption = '...'
    TabOrder = 4
    OnClick = btnSaveDirClick
  end
  object txtUpdateName: TEdit
    Left = 104
    Top = 64
    Width = 121
    Height = 20
    ImeName = #20013#25991' ('#31616#20307') - '#24494#36719#25340#38899
    TabOrder = 5
    Text = 'Update'
    OnChange = txtMainTextChange
  end
  object btnRun: TButton
    Left = 408
    Top = 352
    Width = 75
    Height = 23
    Caption = #25191#34892'(&R)'
    Default = True
    Enabled = False
    TabOrder = 6
    OnClick = btnRunClick
  end
  object btnClose: TButton
    Left = 496
    Top = 352
    Width = 75
    Height = 23
    Cancel = True
    Caption = #20851#38381'(&R)'
    TabOrder = 7
    OnClick = btnCloseClick
  end
  object chkTestFinished: TCheckBox
    Left = 8
    Top = 336
    Width = 145
    Height = 17
    Caption = #20219#21153#23436#25104#26102#27979#35797#25968#25454#21253
    TabOrder = 8
  end
  object txtSubVer: TEdit
    Left = 56
    Top = 32
    Width = 73
    Height = 20
    Enabled = False
    ImeName = #20013#25991' ('#31616#20307') - '#24494#36719#25340#38899
    TabOrder = 9
  end
  object txtPackAdd: TEdit
    Left = 248
    Top = 32
    Width = 281
    Height = 20
    Enabled = False
    ImeName = #20013#25991' ('#31616#20307') - '#24494#36719#25340#38899
    TabOrder = 10
  end
end
