object frmUpdateView: TfrmUpdateView
  Left = 215
  Top = 188
  BorderStyle = bsDialog
  Caption = 'frmUpdateView'
  ClientHeight = 167
  ClientWidth = 344
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
    Width = 60
    Height = 12
    Caption = '  '#20027#29256#26412#65306
  end
  object alabVerFrom: TLabel
    Left = 8
    Top = 36
    Width = 60
    Height = 12
    Caption = #29256#26412'['#20174']'#65306
  end
  object alabPacFileName: TLabel
    Left = 8
    Top = 84
    Width = 60
    Height = 12
    Caption = '  '#25991#20214#21517#65306
  end
  object alabPacFilePath: TLabel
    Left = 8
    Top = 108
    Width = 60
    Height = 12
    Caption = #12288#12288#36335#24452#65306
  end
  object alabVerTo: TLabel
    Left = 8
    Top = 60
    Width = 60
    Height = 12
    Caption = #29256#26412'['#21040']'#65306
  end
  object txtMainVer: TEdit
    Left = 64
    Top = 8
    Width = 73
    Height = 20
    Enabled = False
    ImeName = #20013#25991' ('#31616#20307') - '#24494#36719#25340#38899
    TabOrder = 0
  end
  object txtVerFrom: TEdit
    Left = 64
    Top = 32
    Width = 73
    Height = 20
    Enabled = False
    ImeName = #20013#25991' ('#31616#20307') - '#24494#36719#25340#38899
    TabOrder = 1
  end
  object txtPacFileName: TEdit
    Left = 64
    Top = 80
    Width = 169
    Height = 20
    Enabled = False
    ImeName = #20013#25991' ('#31616#20307') - '#24494#36719#25340#38899
    TabOrder = 2
  end
  object txtPacFilePath: TEdit
    Left = 64
    Top = 104
    Width = 241
    Height = 20
    ImeName = #20013#25991' ('#31616#20307') - '#24494#36719#25340#38899
    TabOrder = 3
    OnChange = txtMainTextChange
  end
  object btnPacFilePath: TButton
    Left = 311
    Top = 104
    Width = 25
    Height = 20
    Caption = '...'
    TabOrder = 4
    OnClick = btnPacFilePathClick
  end
  object btnClose: TButton
    Left = 256
    Top = 136
    Width = 75
    Height = 23
    Cancel = True
    Caption = #21462#28040'(&C)'
    TabOrder = 5
    OnClick = btnCloseClick
  end
  object btnOK: TButton
    Left = 168
    Top = 136
    Width = 75
    Height = 23
    Caption = #30830#23450'(&O)'
    Default = True
    Enabled = False
    TabOrder = 6
    OnClick = btnOKClick
  end
  object txtVerTo: TEdit
    Left = 64
    Top = 56
    Width = 73
    Height = 20
    Enabled = False
    ImeName = #20013#25991' ('#31616#20307') - '#24494#36719#25340#38899
    TabOrder = 7
  end
  object dlgOpen: TOpenDialog
    Filter = #28216#25103#21319#32423#21253'(*.pak)|*.pak'
    Left = 280
    Top = 24
  end
end
