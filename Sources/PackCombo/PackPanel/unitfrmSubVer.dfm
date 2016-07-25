object frmSubVer: TfrmSubVer
  Left = 411
  Top = 210
  BorderStyle = bsDialog
  Caption = 'frmSubVer'
  ClientHeight = 214
  ClientWidth = 332
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
  object alabMainVer: TLabel
    Left = 8
    Top = 12
    Width = 48
    Height = 12
    Caption = #20027#29256#26412#65306
  end
  object alabGameDir: TLabel
    Left = 8
    Top = 72
    Width = 84
    Height = 12
    Caption = #28216#25103#36164#28304#30446#24405#65306
  end
  object Label1: TLabel
    Left = 8
    Top = 112
    Width = 96
    Height = 12
    Caption = #36164#28304#21253#20445#23384#30446#24405#65306
  end
  object Label2: TLabel
    Left = 8
    Top = 32
    Width = 84
    Height = 12
    Caption = #20027#29256#26412#36164#28304#21253#65306
  end
  object alabSubVer: TLabel
    Left = 144
    Top = 12
    Width = 48
    Height = 12
    Caption = #21103#29256#26412#65306
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
  object txtGameDir: TEdit
    Left = 8
    Top = 88
    Width = 281
    Height = 20
    ImeName = #20013#25991' ('#31616#20307') - '#24494#36719#25340#38899
    TabOrder = 1
    OnChange = txtMainTextChange
  end
  object btnGameDir: TButton
    Left = 296
    Top = 88
    Width = 25
    Height = 20
    Caption = '...'
    TabOrder = 2
    OnClick = btnGameDirClick
  end
  object btnRun: TButton
    Left = 72
    Top = 184
    Width = 75
    Height = 23
    Caption = #25191#34892'(&R)'
    Default = True
    Enabled = False
    TabOrder = 3
    OnClick = btnRunClick
  end
  object btnClose: TButton
    Left = 160
    Top = 184
    Width = 75
    Height = 23
    Cancel = True
    Caption = #20851#38381'(&R)'
    TabOrder = 4
    OnClick = btnCloseClick
  end
  object chkTestFinished: TCheckBox
    Left = 8
    Top = 152
    Width = 145
    Height = 17
    Caption = #20219#21153#23436#25104#26102#27979#35797#25968#25454#21253
    TabOrder = 5
  end
  object txtPackSaveFile: TEdit
    Left = 8
    Top = 128
    Width = 281
    Height = 20
    ImeName = #20013#25991' ('#31616#20307') - '#24494#36719#25340#38899
    TabOrder = 6
    OnChange = txtMainTextChange
  end
  object btnPackSaveFile: TButton
    Left = 295
    Top = 128
    Width = 25
    Height = 20
    Caption = '...'
    TabOrder = 7
    OnClick = btnPackSaveFileClick
  end
  object txtPackBase: TEdit
    Left = 8
    Top = 48
    Width = 281
    Height = 20
    Enabled = False
    ImeName = #20013#25991' ('#31616#20307') - '#24494#36719#25340#38899
    TabOrder = 8
  end
  object txtSubVer: TEdit
    Left = 192
    Top = 8
    Width = 73
    Height = 20
    ImeName = #20013#25991' ('#31616#20307') - '#24494#36719#25340#38899
    TabOrder = 9
    OnChange = txtMainTextChange
  end
  object dlgSave: TSaveDialog
    Filter = #28216#25103#36164#28304#21253'(*.pac)|*.pac'
    Left = 296
    Top = 8
  end
end
